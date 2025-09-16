# Use an official base image with Linux
FROM debian:bookworm

# Install g++ and make (for building C++ code)
RUN apt-get update && \
    apt-get install -y g++ make && \
    rm -rf /var/lib/apt/lists/*

# Create a working directory
WORKDIR /app

# Copy your C++ source files into the container
COPY . /app

# Example: compile main.cpp into an executable named "app"


# Run the compiled program by default
CMD ["tail", "-f", "/dev/null"]

