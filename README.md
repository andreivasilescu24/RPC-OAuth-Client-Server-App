# RPC OAuth 

In the server stub, I included the file `database_management.h` to access the function that reads input information from files to initialize the database. The function `load_user_details` will receive as parameters the filenames provided as arguments at runtime, along with their validity, and will store them in data structures/variables that constitute the server's database.

## Implemented Procedures:

- `REQUEST_AUTHORIZATION` will receive from the client the user ID requesting authentication and a flag (0/1) depending on the client's option for automatic access token refresh. The server will check if the received ID exists in the database. The procedure will return a structure containing a field for the auth token and another for a possible error. If the user is not found in the database, the error field will contain the string `USER_NOT_FOUND`, and the token field will be an empty string. Otherwise, the token field will contain the authorization token, and the error field will be empty.

- `REQUEST_ACCESS_TOKEN` will receive the same payload as the authorization procedure, containing the ID and the auto-refresh flag, along with a structure that holds the previously received authorization token. The procedure will return a structure containing the access token (empty in case of an error), the refresh token (empty if auto-refresh was not selected or in case of an error), an error field (empty if access is granted), and a validity field (number of valid operations, 0 in case of an error). The procedure checks whether the end-user has signed the authorization token; if not, it returns the error `REQUEST_DENIED`. If the token is signed, the access token is generated, and if the user has opted for auto-refresh, a refresh token is also generated. The tokens and their validity are returned to the client and saved in the database, mapping the user ID to the access token, refresh token (if applicable), access token to remaining operations, and access token to approved permissions.

- `VALIDATE_DELEGATED_ACTION` will receive a structure containing the action, the resource to be accessed, and the user's access token. It will check whether the token is associated with a user ID (otherwise, it returns the error `PERMISSION_DENIED`), whether the token has remaining valid operations (otherwise, it returns `TOKEN_EXPIRED`), whether the resource exists (otherwise, it returns `RESOURCE_NOT_FOUND`), and whether the token allows the requested operation (otherwise, it returns `OPERATION_NOT_PERMITTED`). If all checks pass, it returns `PERMISSION_GRANTED`. Additionally, if the token exists and has not expired, the number of remaining operations will be updated.

- `APPROVE_REQUEST_TOKEN` will determine the signing status of the authorization token, receiving the token in the payload. The procedure checks if the current request has allocated permissions; if it does, it signs the token and returns it to the client, also storing in the database that the token has been signed. The signing process was implemented by adding a flag in the procedure's response structure, which can be 0 or 1. If the flag is returned to the client with the value 1, it indicates that the token is signed; otherwise, if the value is 0, the token is not signed.

- `REFRESH_TOKEN`: This procedure was added to separate the token refresh logic from the access request logic and to allow sending a different payload containing only the refresh token. This procedure is called only by clients who have opted for auto-refresh. The refresh token is received, a new access token is generated, and the database is updated: the new access token is mapped to the same permissions as the old token, the user ID is mapped to the new access token, and the remaining operations count is reset based on the input validity. A new refresh token is also generated, and the database is updated accordingly. Both generated tokens and their validity are then returned.

## Client

The client reads line by line from the input file provided as an argument at runtime. For each line, it checks whether it is a `REQUEST` operation or another type of operation.

- **`REQUEST` operation:** The client creates the authorization payload with the user ID from the command and the auto-refresh value, then calls the `REQUEST_AUTHORIZATION` procedure. If an error is received, it is displayed, and the process moves to the next command. If an authorization token is received, the client calls the `APPROVE_REQUEST_TOKEN` procedure to approve/sign the previously received token. Then, it calls the `REQUEST_ACCESS_TOKEN` procedure to obtain an access token. If an error is received, it is displayed, and the process continues with the next command. If an access token is received, the client maps it to the user ID from the current command and stores it. Additionally, if the user opted for auto-refresh, the refresh token and the number of remaining valid operations for the access token are stored to determine when a refresh is needed.

- **Other operations:** If the user opted for auto-refresh, the client first checks whether a refresh is needed (i.e., if the current access token has remaining valid operations). If no operations remain, the `REFRESH_TOKEN` procedure is called, updating the tokens for the user ID in the client’s storage. Then, the client creates the payload for action validation, populating it with the operation, resource, and the access token corresponding to the user ID from the command. This payload is sent to the `VALIDATE_DELEGATED_ACTION` procedure. If the access token has expired, the client removes the mapping between the user ID and the current token from storage. If any response other than `PERMISSION_DENIED` is received and auto-refresh is enabled, the remaining operations count for the access token is decremented.

## Server

Besides the functionalities of the procedures described above, here is how the server manages its database:

- The `load_user_details` function reads information from input files and stores the server's resources, users, and corresponding access request permissions. Each request is assigned an index so that the server can allocate permissions to the token based on the number of received requests. Permissions for a resource are represented using a mapping between the resource name and a `ResourceRights` structure, where each action type field can have a value of 0 or 1 (not permitted or permitted). This function is called at server startup from the server stub.

- The server database is managed using multiple maps and vectors, as follows:

  - `resources`: Vector containing the names of available resources on the server.
  - `users`: List of users stored in the server's database.
  - `request_approvals`: Map between request operation ID/counter and the corresponding permissions, helping create the `token_permissions` map.
  - `token_permissions`: Map between access tokens and assigned permissions.
  - `auth_tokens`: Map between user IDs and authorization tokens.
  - `access_tokens`: Map between user IDs and access tokens.
  - `refresh_tokens`: Map between refresh tokens and user IDs.
  - `signed_tokens`: Map storing signed tokens.
  - `token_operations_remaining`: Map between access tokens and their remaining valid operations.

- In the `database_management.cpp` file, various functions are implemented to check database details, update entries, and store the server's database.

