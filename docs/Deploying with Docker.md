# Deploying with Docker

In the root of the repo you'll find a dockerfile. The usual usage applies. 

Build the project then build the docker image. Expose the Data/, Maps/, and Plugins/ folders if you want to persist data.

Server will default to 25565 for the game and 8080 for the webserver unless specified otherwise in the config.