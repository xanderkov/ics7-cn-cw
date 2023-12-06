FROM nvcr.io/nvidia/pytorch:22.06-py3 as builder

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y gcc make

WORKDIR /tmp/server
COPY . .
RUN cmake . && make -j 6

FROM nvcr.io/nvidia/pytorch:22.06-py3
WORKDIR /srv/server
COPY --from=builder /tmp/server/bin/server-thread /srv/server/server-thread




