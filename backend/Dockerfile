# Use official Ubuntu as the base image
FROM ubuntu:22.04

# Install system dependencies
RUN apt-get update && \
    apt-get install -y \
      build-essential \
      cmake \
      git \
      libpqxx-dev \
      libpq-dev \
      libssl-dev \
      wget \
      curl \
      pkg-config \
      libboost-all-dev \
      libcurl4-openssl-dev

# Build and install AWS SDK for C++ (DynamoDB only)
RUN git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp.git && \
    cd aws-sdk-cpp && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY="dynamodb" -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && \
    make install && \
    cd ../.. && rm -rf aws-sdk-cpp

# Set the working directory
WORKDIR /app

# Copy the entire project into the container
COPY . /app

# Build your project
RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# Expose the port your app listens on
EXPOSE 8080

# Set the default command to run your server
CMD ["./build/crow_server"]
