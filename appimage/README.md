To build the AppImage, you basically just have to:

1) Build the base docker container
    a) By doing it manually:
    ```
    docker pull centos:6.8
    docker build .
    docker images (should show the image ID of the newly created image)
    docker run <image id>
    ```
    Once created, you can just detach/attach and start/stop the container, i.e.
    ```
    docker start <container id>
    docker attach <container id>
    ```
    Building the container will take several hours at least.

    b) **OR**: Get Sven's pre-built version:
    ```
    docker pull scummos/centos6.8-qt5.7
    ```

2) Copy the script into the docker container, using docker cp.

3) Run the script.
   Running the script will also take quite a while the first time you do it,
   so it is advisable to always re-use the same container.

4) Copy the resulting AppImage out of the container using docker cp.
