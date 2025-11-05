#!/usr/bin/env zsh
# Generate development certificates
set -euo pipefail

openssl req -x509 -newkey rsa:4096 -keyout certs/dev/server.key \
    -out certs/dev/server.crt -days 365 -nodes \
    -subj '/CN=mithril-dev'

