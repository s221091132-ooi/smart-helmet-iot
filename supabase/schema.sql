-- Smart Helmet IoT System Database Schema
-- Created for Supabase PostgreSQL

-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Table: helmet_data
-- Stores all sensor readings and tracking data from the ESP32
CREATE TABLE IF NOT EXISTS helmet_data (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    
    -- Battery monitoring
    battery_voltage FLOAT NOT NULL,
    battery_percentage INTEGER NOT NULL CHECK (battery_percentage >= 0 AND battery_percentage <= 100),
    remaining_capacity INTEGER NOT NULL CHECK (remaining_capacity >= 0),
    solar_current FLOAT NOT NULL DEFAULT 0,
    
    -- Temperature sensor
    temperature FLOAT NOT NULL,
    
    -- MPU9250 accelerometer data (raw values)
    accel_x INTEGER NOT NULL,
    accel_y INTEGER NOT NULL,
    accel_z INTEGER NOT NULL,
    
    -- MPU9250 gyroscope data (raw values)
    gyro_x INTEGER NOT NULL,
    gyro_y INTEGER NOT NULL,
    gyro_z INTEGER NOT NULL,
    
    -- Location tracking
    position_x FLOAT NOT NULL DEFAULT 0,
    position_y FLOAT NOT NULL DEFAULT 0,
    direction TEXT NOT NULL CHECK (direction IN ('N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW')),
    heading FLOAT NOT NULL DEFAULT 0,
    speed FLOAT NOT NULL DEFAULT 0 CHECK (speed >= 0),
    altitude FLOAT NOT NULL DEFAULT 0,
    distance_traveled FLOAT NOT NULL DEFAULT 0 CHECK (distance_traveled >= 0)
);

-- Index on timestamp for fast queries
CREATE INDEX idx_helmet_data_timestamp ON helmet_data(timestamp DESC);

-- Table: fall_alerts
-- Records fall detection events with location and acceleration data
CREATE TABLE IF NOT EXISTS fall_alerts (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    
    -- Position at time of fall
    position_x FLOAT NOT NULL,
    position_y FLOAT NOT NULL,
    
    -- Acceleration magnitude during impact
    accel_magnitude FLOAT NOT NULL,
    
    -- Alert management
    acknowledged BOOLEAN NOT NULL DEFAULT FALSE
);

-- Index on timestamp and acknowledged status
CREATE INDEX idx_fall_alerts_timestamp ON fall_alerts(timestamp DESC);
CREATE INDEX idx_fall_alerts_acknowledged ON fall_alerts(acknowledged, timestamp DESC);

-- Table: tracking_sessions
-- Manages location tracking origin resets
CREATE TABLE IF NOT EXISTS tracking_sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    started_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    reset_count INTEGER NOT NULL DEFAULT 0,
    last_reset_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Insert initial tracking session
INSERT INTO tracking_sessions (reset_count, last_reset_at)
VALUES (0, NOW())
ON CONFLICT DO NOTHING;

-- Enable Row Level Security (RLS)
ALTER TABLE helmet_data ENABLE ROW LEVEL SECURITY;
ALTER TABLE fall_alerts ENABLE ROW LEVEL SECURITY;
ALTER TABLE tracking_sessions ENABLE ROW LEVEL SECURITY;

-- RLS Policies for helmet_data
-- Allow public INSERT (ESP32 will use API key for authentication)
CREATE POLICY "Allow public insert for helmet_data" ON helmet_data
    FOR INSERT
    WITH CHECK (true);

-- Allow public SELECT (Dashboard reads data)
CREATE POLICY "Allow public select for helmet_data" ON helmet_data
    FOR SELECT
    USING (true);

-- RLS Policies for fall_alerts
-- Allow public INSERT
CREATE POLICY "Allow public insert for fall_alerts" ON fall_alerts
    FOR INSERT
    WITH CHECK (true);

-- Allow public SELECT
CREATE POLICY "Allow public select for fall_alerts" ON fall_alerts
    FOR SELECT
    USING (true);

-- Allow public UPDATE (for acknowledging alerts)
CREATE POLICY "Allow public update for fall_alerts" ON fall_alerts
    FOR UPDATE
    USING (true)
    WITH CHECK (true);

-- RLS Policies for tracking_sessions
-- Allow public INSERT
CREATE POLICY "Allow public insert for tracking_sessions" ON tracking_sessions
    FOR INSERT
    WITH CHECK (true);

-- Allow public SELECT
CREATE POLICY "Allow public select for tracking_sessions" ON tracking_sessions
    FOR SELECT
    USING (true);

-- Allow public UPDATE
CREATE POLICY "Allow public update for tracking_sessions" ON tracking_sessions
    FOR UPDATE
    USING (true)
    WITH CHECK (true);

-- Create a view for latest helmet status (optimization)
CREATE OR REPLACE VIEW latest_helmet_status AS
SELECT 
    hd.*,
    (SELECT COUNT(*) FROM fall_alerts WHERE acknowledged = FALSE) as unacknowledged_falls
FROM helmet_data hd
ORDER BY hd.timestamp DESC
LIMIT 1;

-- Comments for documentation
COMMENT ON TABLE helmet_data IS 'Stores real-time sensor readings and location tracking data from the smart helmet';
COMMENT ON TABLE fall_alerts IS 'Records fall detection events with timestamp and location';
COMMENT ON TABLE tracking_sessions IS 'Manages location tracking origin resets';
COMMENT ON COLUMN helmet_data.battery_voltage IS 'Battery voltage in volts';
COMMENT ON COLUMN helmet_data.battery_percentage IS 'Battery level as percentage (0-100)';
COMMENT ON COLUMN helmet_data.remaining_capacity IS 'Remaining battery capacity in mAh';
COMMENT ON COLUMN helmet_data.solar_current IS 'Solar charging current in amperes';
COMMENT ON COLUMN helmet_data.position_x IS 'X coordinate in meters relative to origin';
COMMENT ON COLUMN helmet_data.position_y IS 'Y coordinate in meters relative to origin';
COMMENT ON COLUMN fall_alerts.accel_magnitude IS 'Acceleration magnitude at impact in m/s²';
