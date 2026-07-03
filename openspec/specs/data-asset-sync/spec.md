## ADDED Requirements

### Requirement: Engine data asset manifest
Phoenix SHALL expose a manifest of the active engine `data` folder for the `pool` and `resources` subtrees.

#### Scenario: Engine manifest is requested
- **WHEN** Cacablu requests the engine asset manifest
- **THEN** Phoenix returns structured entries for files and directories under `data/pool` and `data/resources`
- **AND** each file entry includes a normalized relative path, size, and content hash
- **AND** no entries outside `pool` and `resources` are included

#### Scenario: Engine uses active data folder
- **WHEN** Phoenix was started with the default `data` folder or a `-datafolder` override
- **THEN** the manifest is generated from `DemoKernel::m_dataFolder`

#### Scenario: Repository debug build uses launcher data
- **WHEN** Phoenix is launched from the repository debug build output and no `-datafolder` override is provided
- **THEN** Phoenix resolves the active data folder to the repository `Launcher/data` folder when it exists
- **AND** editor API writes under `pool` and `resources` are physically visible under `Launcher/data`

#### Scenario: Data folder override is provided
- **WHEN** Phoenix is launched with `-datafolder`
- **THEN** Phoenix uses the provided folder for manifests and asset mutations
- **AND** the repository `Launcher/data` default does not override the explicit argument

### Requirement: Editor folder comparison
Cacablu SHALL compare its opened `data` folder against Phoenix's active engine manifest for `pool` and `resources`.

#### Scenario: Project is loaded before comparison
- **WHEN** Cacablu has a project loaded and opens that project's `data` folder
- **THEN** Cacablu may request Phoenix's engine manifest and compare the two asset trees

#### Scenario: No project is loaded
- **WHEN** Cacablu has no project loaded
- **THEN** Cacablu does not start engine asset comparison
- **AND** Cacablu does not transfer files to Phoenix

#### Scenario: Matching folders are opened
- **WHEN** Cacablu opens a `data` folder whose `pool` and `resources` manifest matches Phoenix
- **THEN** Cacablu indicates that the editor and engine asset folders are in sync

#### Scenario: Discrepancy is detected
- **WHEN** Cacablu opens a `data` folder with files missing, extra, or changed relative to Phoenix
- **THEN** Cacablu indicates that there is a discrepancy between the editor and engine asset folders
- **AND** Cacablu identifies the affected relative paths and discrepancy type

#### Scenario: Required subfolders are missing
- **WHEN** Cacablu opens a folder that does not contain `pool` or `resources`
- **THEN** Cacablu reports the missing required subfolder
- **AND** Cacablu does not treat that folder as fully synchronized with Phoenix

### Requirement: Initial published pool synchronization
Cacablu SHALL synchronize enabled project pool files to Phoenix when a project opens.

#### Scenario: Published pool already matches Phoenix
- **WHEN** Cacablu opens a project and Phoenix's `pool` file manifest has exactly the same enabled file paths and sizes
- **THEN** Cacablu does not delete or upload pool files
- **AND** Cacablu may mark those files as already present

#### Scenario: Published pool differs from Phoenix
- **WHEN** Cacablu opens a project and Phoenix's `pool` file manifest has any missing, extra, or size-mismatched file relative to the enabled project files
- **THEN** Cacablu deletes Phoenix's `pool` directory recursively
- **AND** Cacablu recreates `pool`
- **AND** Cacablu uploads every enabled project pool file

#### Scenario: Initial sync is cancelled
- **WHEN** the user cancels the initial project pool synchronization
- **THEN** Cacablu aborts the in-flight request when possible
- **AND** Cacablu does not publish the newly opened project session to the workspace
- **AND** Cacablu remains without a loaded pool for that project

### Requirement: Asset mutation scope
Phoenix SHALL accept editor asset mutations only for paths under `data/pool` and `data/resources`.

#### Scenario: Mutation is sent from loaded project
- **WHEN** Cacablu has a project loaded and sends a valid asset mutation from that project's `data` folder
- **THEN** Phoenix may apply the mutation after path and operation validation succeed

#### Scenario: Mutation is attempted without project context
- **WHEN** Cacablu has no project loaded
- **THEN** Cacablu does not send asset mutations to Phoenix

#### Scenario: Pool file is written
- **WHEN** Cacablu sends a valid create or replace request for a relative path under `pool`
- **THEN** Phoenix writes the file under the active engine `data/pool` subtree

#### Scenario: Resource file is written
- **WHEN** Cacablu sends a valid create or replace request for a relative path under `resources`
- **THEN** Phoenix writes the file under the active engine `data/resources` subtree

#### Scenario: Config file is requested
- **WHEN** Cacablu sends an asset mutation request for a relative path under `config`
- **THEN** Phoenix rejects the request
- **AND** no `config` file is modified

### Requirement: Asset filesystem operations
Phoenix SHALL support explicit create directory, write file, delete file, and delete directory operations for allowed asset paths.

#### Scenario: Directory is created
- **WHEN** Cacablu sends a valid create directory request under `pool` or `resources`
- **THEN** Phoenix creates the directory and any required allowed parent directories

#### Scenario: File is deleted
- **WHEN** Cacablu sends a valid delete file request under `pool` or `resources`
- **THEN** Phoenix deletes that file from the active engine `data` folder

#### Scenario: Directory is deleted
- **WHEN** Cacablu sends a valid delete directory request under `pool` or `resources`
- **THEN** Phoenix deletes that directory only when the request explicitly allows the required delete mode

#### Scenario: Missing target is deleted
- **WHEN** Cacablu sends a delete request for an allowed path that does not exist
- **THEN** Phoenix returns a structured success or no-op result for the requested allowed path
- **AND** no other file is modified

### Requirement: Safe asset writes
Phoenix SHALL write asset file contents in a way that avoids partially written destination files.

#### Scenario: File replacement succeeds
- **WHEN** Cacablu sends a valid file replacement request
- **THEN** Phoenix writes the content to a temporary file in the destination directory
- **AND** Phoenix replaces the destination file only after the temporary write succeeds

#### Scenario: File replacement fails
- **WHEN** a file replacement cannot be completed
- **THEN** Phoenix returns a structured error
- **AND** Phoenix does not leave a partially written destination file

### Requirement: Path validation
Phoenix SHALL validate and normalize every asset path before reading or mutating files.

#### Scenario: Path traversal is requested
- **WHEN** Cacablu sends an asset request containing `..` or otherwise resolving outside the active `data` folder
- **THEN** Phoenix rejects the request
- **AND** no file outside the active `data` folder is read or modified

#### Scenario: Absolute path is requested
- **WHEN** Cacablu sends an absolute filesystem path
- **THEN** Phoenix rejects the request
- **AND** no file is modified

#### Scenario: Backslash path is requested
- **WHEN** Cacablu sends a path using backslashes
- **THEN** Phoenix normalizes the path to forward-slash relative form before validation

### Requirement: Browser-compatible asset API CORS
Phoenix SHALL expose asset HTTP endpoints with browser-compatible CORS headers.

#### Scenario: Cacablu sends a preflight request
- **WHEN** the browser sends an `OPTIONS` preflight for an asset API operation
- **THEN** Phoenix returns one `Access-Control-Allow-Origin` value
- **AND** Phoenix includes allowed methods and `Content-Type` headers
- **AND** Phoenix includes `Access-Control-Allow-Private-Network: true`

#### Scenario: Asset operation response is returned
- **WHEN** Phoenix responds to a create, write, or delete asset operation
- **THEN** the response contains a single valid CORS origin header
- **AND** the browser does not reject the response because of duplicated CORS headers

### Requirement: Asset change events
Phoenix SHALL report asset mutation results to connected Cacablu clients.

#### Scenario: Asset operation succeeds
- **WHEN** Phoenix successfully applies an asset operation
- **THEN** Phoenix emits a structured asset change event over the editor WebSocket
- **AND** the event includes the operation, relative path, and resulting manifest metadata when applicable

#### Scenario: Asset operation fails
- **WHEN** Phoenix rejects or fails an asset operation
- **THEN** Phoenix returns or emits a structured error with a stable code and human-readable message

### Requirement: Runtime reload boundaries
Phoenix SHALL NOT require a full demo reload as part of this asset synchronization capability.

#### Scenario: Asset is changed while loaded
- **WHEN** Cacablu changes a file that Phoenix has already loaded as a shader, texture, model, sound, or other resource
- **THEN** Phoenix applies the filesystem change when valid
- **AND** Phoenix does not promise immediate runtime resource reload as part of this capability
