# Smart Helmet Testing Checklist

This checklist helps you verify all features of the Smart Helmet system are working correctly.

## Pre-Testing Setup

- [ ] ESP32 is properly wired with all sensors
- [ ] Battery is charged to at least 50%
- [ ] WiFi credentials configured in `config.h`
- [ ] Supabase database is set up with schema
- [ ] Web dashboard is deployed to Vercel
- [ ] API endpoint URL is configured in ESP32 code
- [ ] Dashboard environment variables are set

## Hardware Tests

### Power System
- [ ] ESP32 powers on successfully
- [ ] LED can be controlled (turns on during alerts)
- [ ] Buzzer produces sound
- [ ] Battery voltage reads correctly (compare with multimeter)
- [ ] Solar current sensor reads when charging (if applicable)

### Sensors
- [ ] **MPU9250**: Serial Monitor shows "MPU9250 initialized successfully"
- [ ] **DS18B20**: Serial Monitor shows "DS18B20 found: 1 device(s)"
- [ ] **Accelerometer**: Values change when helmet is moved
- [ ] **Gyroscope**: Values change when helmet is rotated
- [ ] **Magnetometer**: Values respond to magnetic field (compass test)
- [ ] **Temperature**: Reads ambient temperature (20-30°C typical)

### WiFi Connectivity
- [ ] ESP32 connects to WiFi automatically
- [ ] Serial Monitor shows "WiFi connected successfully!"
- [ ] IP address is displayed in Serial Monitor
- [ ] WiFi signal strength is adequate (-50 to -70 dBm)
- [ ] Connection remains stable for 5 minutes

## Software Tests

### API Communication
- [ ] ESP32 successfully sends POST request to `/api/helmet/data`
- [ ] Serial Monitor shows "HTTP Response code: 200"
- [ ] No HTTP errors in Serial Monitor
- [ ] Data appears in Supabase `helmet_data` table
- [ ] Timestamp is correct (check timezone)

### Dashboard Connection
- [ ] Dashboard loads without errors
- [ ] Connection status shows "Connected" (green indicator)
- [ ] "Last update" timestamp refreshes every 1 second
- [ ] No errors in browser console (F12)

## Feature Tests

### 1. Battery Monitoring

**Test Steps:**
1. View dashboard battery section
2. Check voltage display
3. Check percentage (0-100%)
4. Check remaining capacity (mAh)
5. If solar panel connected, check charging current

**Pass Criteria:**
- [ ] Voltage matches multimeter reading (±0.1V)
- [ ] Percentage corresponds to voltage (3.0V=0%, 4.2V=100%)
- [ ] Capacity calculation is correct
- [ ] Battery gauge color is appropriate:
  - Green for >30%
  - Yellow for 10-30%
  - Red flashing for <10%
- [ ] Solar current shows >0A when charging

### 2. Temperature Monitoring

**Test Steps:**
1. View dashboard sensor readings section
2. Check temperature value
3. Compare with room thermometer

**Pass Criteria:**
- [ ] Temperature reads within ±2°C of actual
- [ ] Value updates in real-time
- [ ] Reasonable range (15-40°C typically)

### 3. Fall Detection

**Test Steps:**
1. Hold helmet firmly
2. Make quick downward motion (simulate drop)
3. Immediately stop and keep still for 2+ seconds
4. Observe buzzer, LED, and dashboard

**Pass Criteria:**
- [ ] Buzzer sounds continuously (500ms on, 500ms off)
- [ ] LED turns on
- [ ] Red fall alert notification appears on dashboard
- [ ] Alert shows correct timestamp
- [ ] Alert shows position coordinates
- [ ] Acceleration magnitude is >24.5 m/s²
- [ ] Click "Acknowledge" successfully dismisses alert
- [ ] Buzzer stops after acknowledgment
- [ ] Alert marked as acknowledged in database

**False Positive Test:**
- [ ] Normal walking does NOT trigger fall detection
- [ ] Picking up helmet gently does NOT trigger alert
- [ ] Placing helmet down does NOT trigger alert

### 4. Location Tracking

**Test Steps - Initial Setup:**
1. Power on helmet
2. Wait for power-on buzzer pattern (5 beeps, pause, repeat)
3. Open dashboard
4. Click "Reset Location" button
5. Confirm reset
6. Listen for confirmation beep (beep-beep-beeeep)

**Pass Criteria:**
- [ ] Power-on pattern plays correctly
- [ ] Dashboard shows reset confirmation message
- [ ] Helmet plays confirmation sound
- [ ] Map shows position at origin (0, 0)

**Test Steps - Tracking:**
1. Walk forward 10 steps
2. Turn 90° right
3. Walk forward 10 steps
4. Check dashboard map

**Pass Criteria:**
- [ ] Steps are detected (count increases)
- [ ] Position changes on map
- [ ] Path trail shows dots/history
- [ ] Direction indicator updates (N, NE, E, SE, S, SW, W, NW)
- [ ] Speed shows >0 when walking, 0 when stopped
- [ ] Distance traveled increases
- [ ] Position accuracy within ~5-10m after 50m travel

**Direction Test:**
1. Face North (use compass app)
2. Check dashboard direction
3. Rotate 90° to East
4. Check direction updates

**Pass Criteria:**
- [ ] Direction corresponds to actual heading
- [ ] Direction arrow on map points correctly
- [ ] Compass widget shows correct orientation

### 5. Real-Time Updates

**Test Steps:**
1. Open dashboard
2. Walk with helmet
3. Observe update frequency
4. Check "Last update" timestamp

**Pass Criteria:**
- [ ] Dashboard updates every 1 second
- [ ] Timestamp refreshes continuously
- [ ] Battery percentage updates
- [ ] Position updates smoothly
- [ ] Speed updates when walking
- [ ] Temperature updates
- [ ] No lag or freezing
- [ ] Updates continue for 5+ minutes

### 6. Buzzer Patterns

**Test - Power On Pattern:**
1. Power on ESP32
2. Count beeps

**Pass Criteria:**
- [ ] 5 quick beeps (100ms on, 100ms off)
- [ ] 2 second pause
- [ ] Pattern repeats until reset

**Test - Reset Confirmation:**
1. Click "Reset Location" on dashboard
2. Listen to buzzer

**Pass Criteria:**
- [ ] Beep (100ms)
- [ ] Pause (100ms)
- [ ] Beep (100ms)
- [ ] Pause (100ms)
- [ ] Long beep (500ms)

**Test - Fall Alert:**
1. Trigger fall detection
2. Observe buzzer pattern

**Pass Criteria:**
- [ ] Continuous beeping
- [ ] 500ms on, 500ms off
- [ ] Continues until acknowledged

## Integration Tests

### End-to-End Data Flow
1. Move helmet and trigger events
2. Verify data path: ESP32 → API → Database → Dashboard

**Pass Criteria:**
- [ ] Sensor data reaches Supabase within 2 seconds
- [ ] Dashboard shows data within 3 seconds
- [ ] All fields are populated correctly
- [ ] No data loss over 10 minutes
- [ ] Database records have correct timestamps

### Reset Location Synchronization
1. ESP32 is powered and sending data
2. Click "Reset Location" on dashboard
3. Observe helmet response

**Pass Criteria:**
- [ ] Dashboard sends POST request successfully
- [ ] ESP32 receives reset command on next data POST
- [ ] Helmet plays confirmation sound
- [ ] Position resets to (0, 0)
- [ ] Distance traveled resets to 0
- [ ] Step count resets to 0
- [ ] Previous history is cleared

### Multiple Fall Alerts
1. Trigger 3 falls in sequence
2. Check dashboard

**Pass Criteria:**
- [ ] All 3 alerts appear on dashboard
- [ ] Each alert has unique ID
- [ ] Alerts stack vertically
- [ ] Can acknowledge each individually
- [ ] Database records all 3 events

## Performance Tests

### Battery Life Test
**Test Duration:** 1 hour continuous operation

**Pass Criteria:**
- [ ] System runs continuously for 1 hour
- [ ] Battery drains <10% (for 3000mAh battery)
- [ ] No unexpected shutdowns
- [ ] WiFi remains connected
- [ ] Data transmission continues

### WiFi Stability Test
**Test Duration:** 30 minutes

**Pass Criteria:**
- [ ] No disconnections
- [ ] Reconnects automatically if disconnected
- [ ] Data transmission resumes after reconnection
- [ ] No packet loss

### Database Growth Test
**Test Duration:** 1 hour at 1-second intervals

**Pass Criteria:**
- [ ] 3,600 records created (1 per second × 3,600 seconds)
- [ ] All records have complete data
- [ ] No duplicate timestamps
- [ ] Database size increases appropriately (~1-2 MB)

## Stress Tests

### Rapid Movement Test
1. Shake helmet vigorously for 30 seconds
2. Walk quickly with sudden direction changes
3. Check system stability

**Pass Criteria:**
- [ ] No system crashes
- [ ] Sensor readings remain valid
- [ ] No false fall detections
- [ ] Location tracking continues
- [ ] Dashboard remains responsive

### Network Interruption Test
1. System running normally
2. Disconnect WiFi router
3. Wait 30 seconds
4. Reconnect WiFi
5. Observe behavior

**Pass Criteria:**
- [ ] ESP32 detects disconnection
- [ ] Attempts reconnection automatically
- [ ] Reconnects within 10 seconds
- [ ] Data transmission resumes
- [ ] No data loss (or minimal)

## User Experience Tests

### Dashboard Usability
- [ ] Dashboard loads in <3 seconds
- [ ] All components render correctly
- [ ] Text is readable
- [ ] Colors have good contrast
- [ ] Buttons are clickable
- [ ] Responsive on mobile devices
- [ ] Works in Chrome, Firefox, Safari

### Alert Visibility
- [ ] Fall alerts are immediately noticeable
- [ ] Alert sound/animation grabs attention (browser notification)
- [ ] Alert text is clear and informative
- [ ] Acknowledge button is easy to find

## Documentation Tests

### Code Documentation
- [ ] All functions have comments
- [ ] Variable names are descriptive
- [ ] README files are complete
- [ ] Wiring diagrams are accurate

### User Documentation
- [ ] Setup instructions are clear
- [ ] Troubleshooting guide is helpful
- [ ] Testing checklist is comprehensive
- [ ] All features are documented

## Final Sign-Off

### System Integration
- [ ] All hardware components working
- [ ] All software features functional
- [ ] No critical bugs
- [ ] Performance is acceptable
- [ ] Documentation is complete

### Ready for Deployment
- [ ] System tested for 2+ hours continuously
- [ ] All checklist items passed
- [ ] Known issues documented
- [ ] User manual provided

---

## Test Results

**Tester Name:** ________________

**Date:** ________________

**Overall Result:** [ ] PASS  [ ] FAIL  [ ] PARTIAL

**Notes:**
```
(Record any issues, observations, or recommendations here)




```

**Signature:** ________________
