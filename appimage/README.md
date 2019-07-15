To build the AppImage, you basically just have to:

1) Build the base docker image

```
docker pull centos:6.10
docker build <path/to/this/folder/where/Dockerfile>
```
Building the image will take several hours at least.

2) Create a container from image and open a shell
```
docker images (should show the image ID of the newly created image)
docker run -t -i --name kdevelopappimagecreator <image id>
```

3) Copy the script and patches into the docker container
```
docker cp kdevelop-recipe-centos6.sh kdevelopappimagecreator:/
for p in *.patch ; do docker cp $p kdevelopappimagecreator:/ ; done
```

4) Run the script in the interactive shell of the container
```
./kdevelop-recipe-centos6.sh
```

Running the script will also take quite a while the first time you do it,
so it is advisable to always re-use the same container.

5) Copy the resulting AppImage out of the container
```
docker cp kdevelopappimagecreator:/out/KDevelop-git-x86_64.AppImage .
```
