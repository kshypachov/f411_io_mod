# REST API Documentation

## Device and IO Management

- **`GET /api/ram/status`**

  - **Description**: Returns information about free and allocated RAM space.
  - **Response**:
    ```json
    {
        "xAvailableHeapSpaceInBytes": "11688",
        "xSizeOfLargestFreeBlockInBytes": "9528",
        "xSizeOfSmallestFreeBlockInBytes": "48",
        "xNumberOfFreeBlocks": "4",
        "xMinimumEverFreeBytesRemaining": "1976",
        "xNumberOfSuccessfulAllocations": "215",
        "xNumberOfSuccessfulFrees": "180"
    }
    ```

- **`GET /api/io/status`**

  - **Description**: Retrieves the current state of all inputs and outputs.
  - **Response**:
    ```json
    {
      "inputs": [0, 1, 0],
      "outputs": [1, 0, 1]
    }
    ```

- **`POST /api/io/status`**

  - **Description**: Controls output states.
  - **Request Body**:
    ```json
    {
      "relay_id": 1,
      "state": 1
    }
    ```
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Output updated"
    }
    ```

## MQTT Configuration

- **`GET /api/mqtt/settings`**

  - **Description**: Retrieves the current MQTT settings.
  - **Response**:
    ```json
    {
      "enabled": true,
      "broker": "mqtt://broker.example.com",
      "username": "user",
      "password": "******"
    }
    ```

- **`POST /api/mqtt/settings`**

  - **Description**: Updates MQTT configuration settings.
  - **Request Body**:
    ```json
    {
      "enabled": "true",
      "broker": "mqtt://broker.example.com",
      "username": "user",
      "password": "pass"
    }
    ```
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "MQTT settings updated"
    }
    ```

## Device Management

- **`GET /api/device/status`**

  - **Description**: Returns the device's MQTT connection status and IP addresses.
  - **Response**:
    ```json
    {
      "mqtt_conn_status": "connected",
      "local_addr": "192.168.1.10:54678",
      "server_addr": "broker.example.com:1883",
      "pub_topics": [
                "cedar_402258681939/input1/state",
                "cedar_402258681939/input2/state",
                "cedar_402258681939/input3/state",
                "cedar_402258681939/switch1/state",
                "cedar_402258681939/switch2/state",
                "cedar_402258681939/switch3/state"
                    ],
     "subscr_topics": [
                "cedar_402258681939/switch1/set",
                "cedar_402258681939/switch2/set",
                "cedar_402258681939/switch3/set"
                     ]
    }
    ```

- **`POST /api/device/restart`**

  - **Description**: Reboots the device.
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Device is restarting"
    }
    ```

### Logging

- **`GET /api/device/log`** - Fetches the latest device logs.
- **`GET /api/device/log_old`** - Fetches archived logs.

## Firmware Update

- **`POST /api/firmware/upload`**

  - **Description**: Uploads a new firmware file.
  - **Request**: Send `firmware.bin` with method "multiple posts" details [https://mongoose.ws/documentation/tutorials/http/file-uploads/#binary-upload-multiple-posts](https://mongoose.ws/documentation/tutorials/http/file-uploads/#binary-upload-multiple-posts) .
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Firmware uploaded"
    }
    ```

- **`GET /api/firmware/md5`**

  - **Description**: Retrieves the MD5 hash of the uploaded firmware.
  - **Response**:
    ```json
    {
      "status": "success",
      "md5": "5d41402abc4b2a76b9719d911017c592"
    }
    ```

- **`GET /api/firmware/activate`**

  - **Description**: Checks if the firmware is currently activated.
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Firmware activated"
    }
    ```

- **`POST /api/firmware/activate`**

  - **Description**: Activates the uploaded firmware.
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Firmware activated"
    }
    ```

- **`POST /api/firmware/activate_and_reboot`**

  - **Description**: Activates the uploaded firmware and reboot.
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Device is restarting"
    }
    ```

- **`POST /api/firmware/deactivate`**

  - **Description**: Deactivates the uploaded and activated firmware.
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Firmware deactivated"
    }
    ```

## Web Interface Management

- **`POST /api/web_interface/remove`**

  - **Description**: Deletes all files from the web interface directory.
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "All web files deleted"
    }
    ```

- **`POST /api/web_interface/upload`**

  - **Description**: Uploads new files for the web interface.
  - **Request**: Send files with method "multiple posts" details [https://mongoose.ws/documentation/tutorials/http/file-uploads/#binary-upload-multiple-posts](https://mongoose.ws/documentation/tutorials/http/file-uploads/#binary-upload-multiple-posts) .
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Web interface files uploaded"
    }
    ```

- **`POST /api/web_interface/mkdir`**

  - **Description**: Creates a new directory within the web interface dir.
  - **Request Body**:
    ```json
    {
      "name": "new_folder"
    }
    ```
  - **Response**:
    ```json
    {
      "status": "success",
      "message": "Directory created"
    }
    ```

## Web User Management

- **`GET /api/device/user`**

  - **Description**: Retrieve a list of all registered users.
  - **Response**:
    ```json
    {
      "users": [
        { "user": "admin" },
        { "user": "user1" },
        { "user": "user2" }
      ]
    }
    ```

- **`POST /api/device/user`**

  - **Description**: Add a new user to the system.
  - **Request Body**:
    ```json
    {
      "user": "new_user",
      "pass": "new_password"
    }
    ```
  - **Response Body (Success)**:
    ```json
    {
      "status": "success",
      "message": "User added successfully"
    }
    ```
  - **Response Body (Error)**:
    ```json
    {
      "status": "error",
      "message": "Field user or pass is empty"
    }
    ```
  - **Response Body (Error)**:
    ```json
    {
      "status": "error",
      "message": "User list is full"
    }
    ```

- **`PUT /api/device/user`**

  - **Description**: Update the password of an existing user.
  - **Request Body**:
    ```json
    {
      "user": "user",
      "pass": "new_password"
    }
    ```
  - **Response Body (Success)**:
    ```json
    {
      "status": "success",
      "message": "Password updated successfully"
    }
    ```
  - **Response Body (Error)**:
    ```json
    {
      "status": "error",
      "message": "Field user or pass is empty"
    }
    ```
  - **Response Body (Error)**:
    ```json
    {
      "status": "error",
      "message": "User not found"
    }
    ```

- **`DELETE /api/device/user`**

  - **Description**: Delete a user from the system. The user (admin) at index 0 cannot be deleted.
  - **Request Body**:
    ```json
    {
      "user": "user_to_delete"
    }
    ```
  - **Response Body (Success)**:
    ```json
    {
      "status": "success",
      "message": "User deleted successfully"
    }
    ```
  - **Response Body (Error)**:
    ```json
    {
      "status": "error",
      "message": "User not found or cannot delete user at index 0"
    }
    ```
  - **Response Body (Error)**:
    ```json
    {
      "status": "error",
      "message": "Field user is empty or missing"
    }
    ```

## Web Cookies Management

- **`GET, POST /api/auth`**
  - **Description**: Get auth cookies, for auth used Basic Auth method.

  - **Response**: When sending login/password:
    Authorization Cookies.

  - **Response**: When sending authorization cookies:

    ```json
    {
      "status": "success",
      "message": "Token is valid",
      "user": "admin"
    }
    ```

## CORS Support

- ``: Handles CORS preflight requests, allowing cross-origin access for GET, POST, and OPTIONS methods
