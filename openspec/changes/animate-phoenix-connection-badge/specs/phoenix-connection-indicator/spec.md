## ADDED Requirements

### Requirement: Connected Phoenix badge conveys a live connection
Cacablu SHALL render a slowly pulsing green border glow around the Phoenix badge only while the connection status is connected.

#### Scenario: Phoenix is connected and idle
- **WHEN** Cacablu marks Phoenix as connected and no recent communication is occurring
- **THEN** the Phoenix badge has a green border glow
- **AND** its glow pulses slowly while the connection is idle
- **AND** each pulse lights immediately and fades gradually.

#### Scenario: Phoenix disconnects
- **WHEN** the Phoenix status changes from connected to connecting, disconnected, or error
- **THEN** the connected glow pulse stops
- **AND** stale communication activity does not keep the connected animation visible.

### Requirement: Phoenix communication accelerates the indicator
Cacablu SHALL temporarily accelerate the connected glow pulse when it sends or receives Phoenix communication.

#### Scenario: WebSocket communication occurs
- **GIVEN** Phoenix is connected
- **WHEN** Cacablu successfully sends a WebSocket command or receives a valid Phoenix message
- **THEN** the glow pulses faster for a short renewable activity interval
- **AND** entering the activity interval produces an immediate bright pulse
- **AND** returns to its slow idle pulse after communication stops.

#### Scenario: HTTP communication occurs
- **GIVEN** Phoenix is connected
- **WHEN** a Phoenix HTTP request starts or completes
- **THEN** the same activity interval is renewed.

#### Scenario: Repeated communication occurs
- **WHEN** multiple Phoenix messages or requests occur during an existing activity interval
- **THEN** Cacablu renews one activity interval
- **AND** does not create a permanently accelerating or accumulating animation state.

### Requirement: Connection indicator respects reduced motion
Cacablu SHALL preserve connection and activity meaning without pulsing when the user requests reduced motion.

#### Scenario: Reduced motion is enabled
- **GIVEN** the operating system or browser requests reduced motion
- **WHEN** Phoenix is connected
- **THEN** the badge keeps its static green connected glow
- **AND** the glow does not pulse
- **AND** recent activity is represented by a temporary non-motion intensity change.
