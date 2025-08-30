# D3PP REST API Documentation

This document describes the REST API endpoints exposed by the D3PP server plugin (`RestApi.cpp`).  
All endpoints are relative to your server's base URL (default: `http://localhost:8080`).

If you create a directory adjecent to the server called `www`, the server will host that content at the server root (visit http://localhost:8080 in the browser). This can be used to provide a WebApp that interacts with the server.

From the Github repository, a basic React application for controlling and monitoring the server is provided.
I'm not a great front-end dev, so it's quite shitty. Feel free to improve in a PR!

---

## Table of Contents

- [Settings](#settings)
  - [General](#general)
  - [Network](#network)
  - [Text](#text)
  - [Ranks](#ranks)
  - [Blocks](#blocks)
  - [Custom Blocks](#custom-blocks)
  - [Build Modes](#build-modes)
- [World](#world)
  - [Maps](#maps)
- [Network](#network-players)
- [Plugins](#plugins)
- [Player Database](#player-database)
- [System](#system)
- [Error Handling](#error-handling)

---

## Settings

### General

| Method | Path                   | Description                                 |
|--------|------------------------|---------------------------------------------|
| OPTIONS| `/settings/general`    | CORS preflight for general settings.        |
| GET    | `/settings/general`    | Returns general server settings as JSON.    |
| POST   | `/settings/general`    | Updates general server settings.            |

### Network

| Method | Path                   | Description                                 |
|--------|------------------------|---------------------------------------------|
| OPTIONS| `/settings/network`    | CORS preflight for network settings.        |
| GET    | `/settings/network`    | Returns network settings as JSON.           |
| POST   | `/settings/network`    | Updates network settings.                   |

### Text

| Method | Path                   | Description                                 |
|--------|------------------------|---------------------------------------------|
| OPTIONS| `/settings/text`       | CORS preflight for text settings.           |
| GET    | `/settings/text`       | Returns text settings as JSON.              |
| POST   | `/settings/text`       | Updates text settings.                      |

### Ranks

| Method | Path                   | Description                                 |
|--------|------------------------|---------------------------------------------|
| OPTIONS| `/settings/ranks`      | CORS preflight for rank settings.           |
| GET    | `/settings/ranks`      | Returns all ranks as JSON.                  |
| POST   | `/settings/ranks`      | Updates all ranks from JSON.                |
| DELETE | `/settings/ranks/{id}` | Deletes the rank with the specified ID.     |

### Blocks

| Method | Path                   | Description                                 |
|--------|------------------------|---------------------------------------------|
| OPTIONS| `/settings/blocks`     | CORS preflight for block settings.          |
| GET    | `/settings/blocks`     | Returns all blocks as JSON.                 |
| POST   | `/settings/blocks`     | Updates all blocks from JSON.               |
| DELETE | `/settings/blocks/{id}`| Deletes the block with the specified ID.    |

### Custom Blocks

| Method | Path                        | Description                                |
|--------|-----------------------------|--------------------------------------------|
| GET    | `/settings/customblocks`    | (Reserved/Not implemented)                 |

### Build Modes

| Method | Path                        | Description                                |
|--------|-----------------------------|--------------------------------------------|
| GET    | `/settings/buildmodes`      | (Reserved/Not implemented)                 |

---

## World

### Maps

| Method | Path                | Description                                         |
|--------|---------------------|-----------------------------------------------------|
| OPTIONS| `/world/maps`       | CORS preflight for maps.                            |
| GET    | `/world/maps`       | Returns a list of all loaded maps (id, name, etc).  |
| POST   | `/world/maps`       | Creates a new map. Requires `x`, `y`, `z`, `name`.  |

---

## Network Players

| Method | Path                | Description                                         |
|--------|---------------------|-----------------------------------------------------|
| GET    | `/network/players`  | Returns current/max player count and player names.  |

---

## Plugins

| Method | Path                | Description                                         |
|--------|---------------------|-----------------------------------------------------|
| GET    | `/plugins/`         | (Reserved/Not implemented)                          |

---

## Player Database

| Method | Path                | Description                                         |
|--------|---------------------|-----------------------------------------------------|
| GET    | `/playerdb`         | (Reserved/Not implemented)                          |

---

## System

| Method | Path                | Description                                         |
|--------|---------------------|-----------------------------------------------------|
| GET    | `/system/log`       | Returns the last 500 log messages as JSON.          |
| OPTIONS| `/system/message`   | CORS preflight for system message.                  |
| POST   | `/system/message`   | Broadcasts a system message to all clients.         |

---

## Error Handling

- All endpoints set `Access-Control-Allow-Origin: http://localhost:3000` for CORS.
- All POST endpoints expect JSON request bodies unless otherwise specified.
- All responses are in JSON format.
- Error responses will include an HTTP status code and a message.
- Unhandled errors return an HTML error page with the status code and message.

---

**Note:**  
Some endpoints (such as `/settings/customblocks`, `/settings/buildmodes`, `/plugins/`, `/playerdb`) are placeholders and may not be implemented