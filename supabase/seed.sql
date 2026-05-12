-- Optional seed data for testing the Smart Helmet system
-- This file provides sample data for development and testing

-- Insert sample helmet data (simulating 10 data points over time)
INSERT INTO helmet_data (
    timestamp,
    battery_voltage,
    battery_percentage,
    remaining_capacity,
    solar_current,
    temperature,
    accel_x, accel_y, accel_z,
    gyro_x, gyro_y, gyro_z,
    position_x, position_y,
    direction,
    speed,
    altitude,
    distance_traveled
) VALUES
    (NOW() - INTERVAL '10 seconds', 4.15, 85, 2550, 0.25, 28.5, 150, -20, 16380, 5, -2, 10, 0.0, 0.0, 'N', 0.0, 100.0, 0.0),
    (NOW() - INTERVAL '9 seconds', 4.14, 84, 2520, 0.24, 28.6, 200, -15, 16350, 8, -3, 12, 0.7, 0.0, 'N', 2.5, 100.1, 0.7),
    (NOW() - INTERVAL '8 seconds', 4.14, 84, 2490, 0.23, 28.7, 180, -10, 16400, 10, -1, 15, 1.4, 0.0, 'N', 2.5, 100.2, 1.4),
    (NOW() - INTERVAL '7 seconds', 4.13, 83, 2460, 0.22, 28.8, 160, -5, 16380, 6, 0, 13, 2.1, 0.0, 'N', 2.5, 100.3, 2.1),
    (NOW() - INTERVAL '6 seconds', 4.13, 83, 2430, 0.21, 28.9, 170, 0, 16390, 4, 2, 11, 2.8, 0.0, 'N', 2.5, 100.4, 2.8),
    (NOW() - INTERVAL '5 seconds', 4.12, 82, 2400, 0.20, 29.0, 190, 5, 16370, 7, 4, 14, 3.3, 0.5, 'NE', 3.0, 100.5, 3.6),
    (NOW() - INTERVAL '4 seconds', 4.12, 82, 2370, 0.19, 29.1, 200, 10, 16360, 9, 6, 16, 3.8, 1.0, 'NE', 3.2, 100.6, 4.5),
    (NOW() - INTERVAL '3 seconds', 4.11, 81, 2340, 0.18, 29.2, 210, 15, 16350, 11, 8, 18, 4.3, 1.5, 'NE', 3.5, 100.7, 5.4),
    (NOW() - INTERVAL '2 seconds', 4.11, 81, 2310, 0.17, 29.3, 220, 20, 16340, 13, 10, 20, 4.8, 2.0, 'NE', 3.8, 100.8, 6.4),
    (NOW() - INTERVAL '1 second', 4.10, 80, 2280, 0.16, 29.4, 230, 25, 16330, 15, 12, 22, 5.3, 2.5, 'E', 4.0, 100.9, 7.5);

-- Insert sample fall alerts (2 falls, one acknowledged, one not)
INSERT INTO fall_alerts (
    timestamp,
    position_x,
    position_y,
    accel_magnitude,
    acknowledged
) VALUES
    (NOW() - INTERVAL '5 minutes', 10.5, 8.3, 28.7, TRUE),
    (NOW() - INTERVAL '30 seconds', 5.3, 2.5, 26.2, FALSE);

-- Initialize tracking session with one reset
UPDATE tracking_sessions
SET 
    reset_count = 1,
    last_reset_at = NOW() - INTERVAL '10 seconds'
WHERE id = (SELECT id FROM tracking_sessions LIMIT 1);

-- Display confirmation
SELECT 'Seed data inserted successfully!' as status;
SELECT COUNT(*) as helmet_data_count FROM helmet_data;
SELECT COUNT(*) as fall_alerts_count FROM fall_alerts;
SELECT * FROM tracking_sessions;
