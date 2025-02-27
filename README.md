# RPC OAuth Client Server Application 

## Server Stub

In the server stub, I included the file `database_management.h` to access the function that reads input information from files to initialize the database. The function `load_user_details` will receive as parameters the filenames provided as arguments at runtime, along with their validity, and will store them in data structures/variables that constitute the server's database.

## Implemented Procedures

- **`REQUEST_AUTHORIZATION`**: Receives user ID and auto-refresh flag from the client, checks database existence, and returns an authorization token or an error.
- **`REQUEST_ACCESS_TOKEN`**: Validates the signed authorization token, issues an access token, and optionally generates a refresh token. Updates the database accordingly.
- **`VALIDATE_DELEGATED_ACTION`**: Verifies if the access token is valid, has remaining operations, and grants or denies permission based on stored rules.
- **`APPROVE_REQUEST_TOKEN`**: Signs the authorization token if appropriate permissions are allocated, updating the database.
- **`REFRESH_TOKEN`**: Handles token refresh logic separately, issuing new access and refresh tokens while maintaining permissions.

## Client

- **Handles authentication requests** by generating and managing tokens, ensuring secure storage and tracking token expiration.
- **Manages delegated actions** by verifying user permissions before executing operations, refreshing tokens if necessary.
- **Maintains local mappings** for user IDs and tokens to streamline request processing and optimize interactions with the server.

## Server

Besides the functionalities of the procedures described above, here is how the server manages its database:

- **Loads user and resource details** from input files, assigning permissions to requests dynamically.
- **Uses structured mappings** for efficient database management, including authentication and access control records.
- **Implements robust data storage** with multiple maps and vectors, optimizing user access verification and security enforcement.

## Database Structure

- **`resources`**: Vector of available resource names.
- **`users`**: List of registered users.
- **`request_approvals`**: Tracks permissions granted to access requests.
- **`token_permissions`**: Maps access tokens to assigned permissions.
- **`auth_tokens`**: Links user IDs to authorization tokens.
- **`access_tokens`**: Links user IDs to access tokens.
- **`refresh_tokens`**: Associates refresh tokens with user IDs.
- **`signed_tokens`**: Stores signed authorization tokens.
- **`token_operations_remaining`**: Tracks valid operations per access token.

In the `database_management.cpp` file, various functions check, update, and maintain database entries to support the authentication and authorization system efficiently.


