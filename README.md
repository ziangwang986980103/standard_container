test process:
1. cd into the standard_container directory and run the linux docker by running "docker run -it --rm -v $(pwd):/work -w /work gcc:13 bash"
2. cd into the vector container and run "./tests.sh"