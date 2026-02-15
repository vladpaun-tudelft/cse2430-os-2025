# cse2430-os-2025

TU Delft CSE2430 Operating Systems lab assignments.

## Docker workflow

This repo includes:
- `Dockerfile` for the development image
- `docker-compose.yml` for a repeatable interactive dev container

### Start an interactive shell

```bash
docker compose run --rm dev
```

This is equivalent to the earlier manual flow:
- `docker build --tag cse2430-image .`
- `docker run -it -v ./:/home/assignments cse2430-image`

Compose just stores those options in one file so you do not need to remember them.

### Why mount the repo

The compose file mounts the repository into the container:
- host: `./`
- container: `/home/assignments`

That means edits persist on your machine and are visible both inside and outside the container.

## Repo layout recommendation

Keep a single `Dockerfile` and `docker-compose.yml` at the repo root for all six assignments.

Reason:
- all assignments use the same C/Linux tooling
- one shared environment avoids duplicated config and drift
- each assignment can stay isolated in its own folder (`assignment1` ... `assignment6`)

If a later assignment needs extra dependencies, update the root image once (or add an optional compose override), instead of copying setup into every assignment directory.
