## 1. Phoenix Activity Signal

- [x] 1.1 Add a small shared Phoenix activity publisher/subscriber with a fetch wrapper that signals request start and completion.
- [x] 1.2 Route all Phoenix HTTP clients through the activity-aware fetch boundary.
- [x] 1.3 Signal valid incoming and successfully sent WebSocket communication from the connection controller.
- [x] 1.4 Add unit tests for activity publication, subscription cleanup, and HTTP success/failure completion signals.

## 2. Connection Badge Presentation

- [x] 2.1 Subscribe the shell badge to Phoenix activity with one renewable timeout and clear transient activity outside the connected state.
- [x] 2.2 Add the connected green glow, slow idle pulse, faster activity state, and disconnected/error cleanup styles.
- [x] 2.3 Add reduced-motion styles that disable pulsing while retaining static connection and activity emphasis.
- [x] 2.4 Remove the rotating halo and replace it with slow and activity-speed glow pulses.
- [x] 2.5 Make idle and activity pulses illuminate immediately and fade gradually, restarting the visual pulse when activity begins.

## 3. Verification

- [x] 3.1 Add browser coverage for connected idle, active communication, timeout recovery, disconnected state, and reduced motion.
- [x] 3.2 Run typecheck, unit tests, targeted lint, production build, and strict OpenSpec validation.
- [x] 3.3 Update browser coverage for the idle and active pulse speeds and rerun focused verification.
- [x] 3.4 Verify the asymmetric pulse shape, immediate activity animation, and reduced-motion override.
