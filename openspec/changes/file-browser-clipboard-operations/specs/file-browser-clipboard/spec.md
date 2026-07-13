## ADDED Requirements

### Requirement: Pool items participate in context-aware clipboard commands
Cacablu SHALL route Cut, Copy, and Paste to the Pool browser when it owns the command and SHALL preserve native editing behavior for text destinations.

#### Scenario: Copy selected Pool file
- **GIVEN** the Pool panel is active and a file is selected
- **WHEN** the user invokes Copy from the keyboard or Edit menu
- **THEN** Cacablu captures the selected file for an internal asset paste
- **AND** writes its normalized `/pool/...` path to the system clipboard as plain text.

#### Scenario: Copy selected Pool folder
- **GIVEN** the Pool panel is active and a folder is selected
- **WHEN** the user invokes Copy
- **THEN** Cacablu captures the complete folder subtree for an internal asset paste
- **AND** writes the selected folder's normalized `/pool/...` path as plain text.

#### Scenario: Copy multiple selected Pool items
- **GIVEN** multiple files or folders are selected in the Pool browser
- **WHEN** the user invokes Copy or Cut
- **THEN** Cacablu captures every canonical selected root in visible-tree order
- **AND** a selected descendant of another selected folder is not captured twice.

#### Scenario: Clipboard command occurs in a text editor
- **GIVEN** focus is inside Monaco, an input, textarea, select, or contenteditable element
- **WHEN** the user invokes Cut, Copy, or Paste
- **THEN** Cacablu leaves the command to the text editor's native behavior
- **AND** the Pool selection is not copied, moved, or deleted.

#### Scenario: Pool browser cannot claim a command
- **WHEN** no valid Pool item or paste destination is available
- **THEN** Cacablu leaves the project unchanged
- **AND** does not report a successful Pool clipboard operation.

### Requirement: Text destinations receive Phoenix Pool paths
Cacablu SHALL publish selected asset paths as a plain-text clipboard representation suitable for Phoenix scripts.

#### Scenario: File path is pasted into text
- **GIVEN** the user copied or cut `pool/textures/hero.png` from the Pool browser
- **WHEN** the user pastes into Monaco or another text field
- **THEN** the inserted text is `/pool/textures/hero.png`.

#### Scenario: Folder path is pasted into text
- **GIVEN** the user copied or cut a selected Pool folder
- **WHEN** the user pastes into a text destination
- **THEN** the inserted text starts with `/pool/`
- **AND** uses forward slashes with no trailing slash.

#### Scenario: Multiple paths are pasted into text
- **GIVEN** multiple canonical Pool roots were copied or cut
- **WHEN** the user pastes into a text destination
- **THEN** the inserted text contains one normalized `/pool/...` path per line
- **AND** preserves the visible-tree order of the copied roots.

#### Scenario: Cut path is pasted into text
- **GIVEN** a Pool item is pending Cut
- **WHEN** its plain-text path is pasted outside the Pool browser
- **THEN** Cacablu does not move or delete the source item
- **AND** the pending internal cut remains available until a valid Pool paste or invalidation.

#### Scenario: File is dragged into a code editor
- **GIVEN** a Pool file is dragged from the File Browser
- **WHEN** the user drops it into Monaco or another text destination
- **THEN** the editor inserts the file's normalized `/pool/...` path at the drop position
- **AND** Cacablu does not move or copy the project file.

#### Scenario: Platform lacks native file-list clipboard support
- **WHEN** the browser cannot publish a native filesystem file-list representation
- **THEN** Copy and Cut still publish the `/pool/...` plain-text representation
- **AND** internal Pool paste remains available
- **AND** Cacablu does not claim that the item can be pasted as a file into the operating-system desktop.

### Requirement: Copied Pool items can be pasted recursively
Cacablu SHALL copy clipboard files or folders into the resolved Pool destination without changing the sources.

#### Scenario: Paste a copied file into another folder
- **GIVEN** a Pool file was copied
- **AND** a different destination folder is selected
- **WHEN** the user invokes Paste
- **THEN** Cacablu creates a new file in the destination with the same name, bytes, type, format, and enabled state
- **AND** leaves the source file unchanged
- **AND** keeps the copy available for another paste.

#### Scenario: Paste a copied folder
- **GIVEN** a Pool folder was copied
- **WHEN** the user pastes it into a valid destination
- **THEN** Cacablu recreates the selected folder and its complete descendant hierarchy
- **AND** preserves every contained file's contents, metadata, and enabled state
- **AND** assigns new database ids to the copied rows.

#### Scenario: Paste multiple copied roots
- **GIVEN** multiple canonical Pool roots were copied
- **WHEN** the user pastes them into a valid destination
- **THEN** Cacablu creates every planned root and subtree as one local operation
- **AND** leaves every source unchanged.

#### Scenario: Resolve paste destination
- **WHEN** the user invokes Pool Paste
- **THEN** a selected folder is the destination
- **AND** if a file is selected its parent folder is the destination
- **AND** if no item is selected the Pool root is the destination.

#### Scenario: Paste into the explicit Pool root
- **GIVEN** the Pool root row is selected
- **WHEN** the user invokes Paste for copied or cut Pool items
- **THEN** Cacablu uses the Pool root as the destination
- **AND** creates or moves the pasted roots with parent id `0`.

#### Scenario: Drop a file onto the Pool root
- **WHEN** the user drops an internal Pool file or an external file onto the Pool root
- **THEN** Cacablu uses the Pool root as the destination
- **AND** applies the same conflict, persistence, and Phoenix synchronization behavior used for folder drop targets.

#### Scenario: Multiple selected rows make the destination ambiguous
- **WHEN** the user invokes Paste while multiple Pool rows are selected
- **THEN** Cacablu does not guess a destination
- **AND** asks the user to select one destination folder or file context.

#### Scenario: Destination contains a conflicting name
- **GIVEN** the destination already contains a sibling file or folder with the copied root item's name, compared case-insensitively
- **WHEN** the user invokes Paste
- **THEN** Cacablu rejects the operation before any project row changes
- **AND** reports the conflict to the user.

### Requirement: Cut Pool items move only on successful internal paste
Cacablu SHALL treat Cut as one or more deferred moves and consume them only after a valid Pool paste succeeds.

#### Scenario: Cut and paste a file
- **GIVEN** a Pool file is pending Cut
- **WHEN** the user pastes it into a valid different folder
- **THEN** Cacablu moves the existing file to the destination
- **AND** preserves its database id, contents, metadata, and enabled state
- **AND** clears the pending cut after success.

#### Scenario: Cut and paste a folder
- **GIVEN** a Pool folder is pending Cut
- **WHEN** the user pastes it into a valid destination outside its subtree
- **THEN** Cacablu moves the existing root folder
- **AND** preserves descendant ids and relationships
- **AND** clears the pending cut after success.

#### Scenario: Cut and paste multiple roots
- **GIVEN** multiple canonical Pool roots are pending Cut
- **WHEN** the user pastes them into a valid destination
- **THEN** Cacablu moves all roots as one local operation
- **AND** clears the complete pending cut only after success.

#### Scenario: Folder is pasted into itself or a descendant
- **GIVEN** a Pool folder is pending Cut
- **WHEN** its resolved destination is itself or one of its descendants
- **THEN** Cacablu rejects the move before any project row changes
- **AND** retains the pending cut for a different destination.

#### Scenario: Cut source becomes stale
- **GIVEN** the cut source was deleted or the active project session changed
- **WHEN** the user invokes Paste
- **THEN** Cacablu does not mutate the current project
- **AND** invalidates the stale pending cut
- **AND** reports that the source is no longer available.

### Requirement: Pending cut items have a temporary visual state
Cacablu SHALL render canonical cut roots at 50% opacity until the pending cut is consumed or invalidated.

#### Scenario: Cut marks selected rows
- **WHEN** the user cuts one or more Pool items successfully
- **THEN** each canonical cut-root row is rendered at 50% opacity
- **AND** the rows remain selectable and operable.

#### Scenario: Internal paste completes
- **GIVEN** Pool rows are shown as pending Cut
- **WHEN** their internal paste succeeds
- **THEN** Cacablu clears the pending cut
- **AND** removes the 50%-opacity styling.

#### Scenario: Cacablu clipboard content is replaced
- **GIVEN** Pool rows are shown as pending Cut
- **WHEN** another Copy or Cut command in Cacablu replaces the clipboard content
- **THEN** Cacablu removes the pending-cut styling immediately.

#### Scenario: External clipboard replacement becomes observable
- **GIVEN** Pool rows are shown as pending Cut
- **AND** another application replaced the system clipboard
- **WHEN** Cacablu observes the replacement through a clipboard event or a permitted focus/visibility revalidation
- **THEN** Cacablu invalidates the pending cut
- **AND** removes the 50%-opacity styling.

#### Scenario: Project session changes
- **GIVEN** Pool rows are shown as pending Cut
- **WHEN** the active project session closes or changes
- **THEN** Cacablu clears the pending cut and its visual styling.

### Requirement: Clipboard mutations synchronize enabled assets with Phoenix
Cacablu SHALL keep Phoenix asset paths consistent with successful local clipboard operations using existing scoped asset APIs.

#### Scenario: Copy succeeds while Phoenix is connected
- **WHEN** a copied file or folder is pasted successfully
- **THEN** Cacablu writes every enabled destination file to its new Phoenix `/pool` path
- **AND** does not publish disabled destination files.

#### Scenario: Cut succeeds while Phoenix is connected
- **WHEN** a cut file or folder is pasted successfully
- **THEN** Cacablu writes enabled files at their destination paths
- **AND** removes their previous enabled Phoenix paths only after destination writes are attempted.

#### Scenario: Phoenix is disconnected
- **WHEN** a local clipboard copy or move succeeds while Phoenix is disconnected
- **THEN** Cacablu keeps the successful project mutation
- **AND** marks the project dirty
- **AND** does not attempt a Phoenix asset request.

#### Scenario: Phoenix reconciliation fails
- **WHEN** Phoenix rejects an asset write or delete after the local clipboard mutation succeeds
- **THEN** Cacablu retains the project mutation as the source of truth
- **AND** records the discrepancy in Events
- **AND** a later full project synchronization can repair Phoenix.

### Requirement: Clipboard commands do not partially mutate project data
Cacablu SHALL validate and commit each internal paste as one local operation.

#### Scenario: Recursive copy validation fails
- **WHEN** any source, destination, cycle, or name validation fails before a folder copy
- **THEN** Cacablu inserts no folders or files
- **AND** leaves the project dirty state unchanged.

#### Scenario: Database mutation fails
- **WHEN** the database cannot complete a planned copy or move
- **THEN** Cacablu rolls back that local clipboard mutation
- **AND** does not begin Phoenix reconciliation for it.
