REPO := $(abspath ..)
.PHONY: 3ds
3ds:
	docker run --rm -v "$(REPO):/repo" -w /repo devkitpro/devkitarm:latest bash /repo/gpsp3ds/build-companion.sh
