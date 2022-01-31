#!/usr/bin/env bash
#
# Author: Bhaskar K C

_sha256() {
    echo 'Q' \
        | openssl s_client \
        -connect "${SERVER}":443 \
        2> /dev/null \
        | openssl x509 -pubkey -noout \
        | openssl pkey -pubin -outform der \
        | openssl dgst -sha256 -binary \
        | openssl enc -base64
}


gvpn() {
    echo -n "${PASS}" \
    | openconnect --protocol=gp "${SERVER}" \
        --servercert pin-sha256:"$(_sha256)" \
        --user="${USER}" -b && echo "Connected!" && \
        /bin/bash
}


kaiju_connect() {

    sshpass -p ${KAIJUPASSWD} ssh -tt -oStrictHostKeyChecking=no ${KAIJUUSER}@kaiju.anu.edu.au
    mkdir this_was_created_by_action

}

gvpn && kaiju_connect
