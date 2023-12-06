FROM ubuntu as builder

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y gcc cmake make

WORKDIR /tmp/server
COPY . .
RUN cmake . && make -j 6

FROM ubuntu
WORKDIR /srv/server
COPY --from=builder /tmp/server/bin/server-thread /srv/server/server-thread




