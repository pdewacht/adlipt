FROM alpine:3.9

# Download and install OpenWatcom 1.9. It's only distributed as an interactive
# installer, but it turns out you can just unzip the file. Afterwards you just
# need to fix up the permissions.

RUN apk add --no-cache curl unzip

RUN \
    curl -O ftp://ftp.openwatcom.org/install/open-watcom-c-linux-1.9 && \
    echo 'f7484be27eb70028010303fc16bb2acc5a785679567a568b940c28190ddbf3f3 *open-watcom-c-linux-1.9' | sha256sum -c && \
    unzip -q open-watcom-c-linux-1.9 -d /watcom && \
    rm open-watcom-c-linux-1.9 && \
    cd /watcom/binl && chmod +x \
        whelp wasaxp wcl wlink parserv exe2bin wcc386 wasm ctags owcc \
        ide2make wdump wd tcpserv wdis dmpobj wpp wpp386 wmake vi wstrip \
        wcc wbind edbind wlib fcenable ms2wlink wtouch wipfc wasppc wrc wcl386

# Watcom environment variables

ENV WATCOM /watcom
ENV INCLUDE /watcom/h
ENV PATH /watcom/binl:$PATH

# Tools needed for ADPATCH

RUN apk add --no-cache python3 py3-yaml ragel nasm zip
