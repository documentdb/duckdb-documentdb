#!/usr/bin/env bash
set -euo pipefail

documentdb_image="ghcr.io/documentdb/documentdb/documentdb-local:latest"
documentdb_container="duckdb-documentdb-e2e-db"
client_image="duckdb-documentdb-e2e-client"
network="duckdb-documentdb-e2e"
username="duckdb_e2e_admin"
password="E2e$(date +%s)${RANDOM}${RANDOM}Aa1!"

cleanup() {
    docker rm --force "${documentdb_container}" >/dev/null 2>&1 || true
    docker network rm "${network}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

cleanup
docker network create "${network}" >/dev/null
docker run --detach --name "${documentdb_container}" --network "${network}" \
    "${documentdb_image}" --username "${username}" --password "${password}" --skip-init-data >/dev/null

ready=false
for _ in $(seq 1 80); do
    if docker exec "${documentdb_container}" mongosh localhost:10260 \
        -u "${username}" -p "${password}" --authenticationDatabase admin \
        --authenticationMechanism SCRAM-SHA-256 --tls --tlsAllowInvalidCertificates \
        --quiet --eval 'db.runCommand({ping:1}).ok' >/dev/null 2>&1; then
        ready=true
        break
    fi
    sleep 3
done

if [[ "${ready}" != "true" ]]; then
    echo "DocumentDB Local did not become ready" >&2
    docker logs "${documentdb_container}" >&2
    exit 1
fi

seed='const c=db.getSiblingDB("duckdb_e2e").getCollection("orders\"archive"); c.drop(); c.insertMany([{status:"active",total:99},{status:"pending",total:49}]);'
docker exec "${documentdb_container}" mongosh localhost:10260 \
    -u "${username}" -p "${password}" --authenticationDatabase admin \
    --authenticationMechanism SCRAM-SHA-256 --tls --tlsAllowInvalidCertificates \
    --quiet --eval "${seed}" >/dev/null

docker build --file tests/Dockerfile.e2e --tag "${client_image}" .
docker run --rm --network "${network}" \
    --env DOCUMENTDB_HOST="${documentdb_container}" \
    --env DOCUMENTDB_USER="${username}" \
    --env DOCUMENTDB_PASSWORD="${password}" \
    "${client_image}"

echo "DocumentDB Docker E2E test passed."