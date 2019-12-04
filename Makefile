image:
	docker build -t aguegu/sdcc .

push:
	docker push aguegu/sdcc

.PHONY: image push
