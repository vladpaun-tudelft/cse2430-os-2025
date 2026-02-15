# Build an image from this Dockerfile using `docker build --tag 'cse2430-image' .`
# Then you can run it with `docker run -i -t 'cse2430-image'`
#
# You may find it useful to mount a directory in the container, so that the files
# are mirrored on your PC and not removed when the container is destroyed. e.g. to
# mount the directory `assignments` in /home/assignments in the image, use
# `docker run -v ./assignments:/home/assignments -i -t 'cse2430-image'`

FROM python:3.13-bookworm

# Install packages
RUN apt-get update && apt-get install -y \
      sudo \
      man \
      manpages-dev \
      bash \
      make \
      build-essential \
      libssl-dev \
      libreadline-dev \
      coreutils \
      strace \
      libxxhash-dev \
      libfuse-dev \
      gdb \
      nano \
      vim \
      less \
    --no-install-recommends \
 && rm -r /var/lib/apt/lists/*

# Default entrypoint
CMD ["bash"]
