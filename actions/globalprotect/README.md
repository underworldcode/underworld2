# Docker container for GlobalProtect

## Steps to build

- `docker build -t gp .`
- ```sh
  docker run --privileged -it \
      -e SERVER=<globalprotect-host-url> \
      -e USER=<username> \
      -e PASSWORD=<password> \
      gp
  ```
# vpn_action
