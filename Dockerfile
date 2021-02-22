# Build application

FROM ubuntu:latest as build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    gcc \
    g++ \
    cmake \
    python3-pip \
    && \
    pip3 install conan

ADD . /app

WORKDIR /app/conandir

RUN conan install ..

WORKDIR /app/build

RUN cmake .. && \
    cmake --build .

# Run application

FROM ubuntu:latest

RUN groupadd -r sample && useradd -r -g sample sample
USER sample

WORKDIR /app

COPY --from=build /app/build/bin/http-server .

ENTRYPOINT [ "/app/http-server", "0.0.0.0", "8019", "." ]

EXPOSE 8019

