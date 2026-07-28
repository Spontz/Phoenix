## ADDED Requirements

### Requirement: Thread-safe sound seek and decode
Phoenix SHALL synchronize all per-sound decoder operations so seeking, decoding, and unloading do not race across threads.

#### Scenario: Seek while callback is decoding
- **WHEN** the main thread calls `seekSound()` while the audio callback is reading frames
- **THEN** decoder access SHALL be serialized
- **AND** Phoenix SHALL NOT read from a freed or concurrently-mutated decoder instance.

#### Scenario: Stop at end-of-file after synchronized read
- **WHEN** a callback decode pass reads fewer frames than requested
- **THEN** the sound SHALL transition to stopped playback state
- **AND** no unsynchronized decoder access SHALL occur during that transition.

### Requirement: Thread-safe shared sound list iteration
Phoenix SHALL prevent races between callback iteration and sound-list mutation.

#### Scenario: Clear or reload sounds during runtime
- **WHEN** runtime code clears or reloads sounds while playback callback is active
- **THEN** callback iteration SHALL operate on a safe snapshot or equivalent synchronized view
- **AND** Phoenix SHALL NOT access invalidated sound-list iterators or entries.

### Requirement: Preserve existing seek semantics
Phoenix SHALL keep current seek behavior while adding synchronization.

#### Scenario: Seek by section runtime
- **WHEN** a sound section initializes and seeks to its section runtime
- **THEN** Phoenix SHALL seek using seconds converted to decoder PCM frame position
- **AND** subsequent playback SHALL continue from that target position.
