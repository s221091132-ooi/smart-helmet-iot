// lib/types.ts - TypeScript types for Smart Helmet system

// Database type definitions
export interface Database {
  public: {
    Tables: {
      helmet_data: {
        Row: HelmetDataRow;
        Insert: HelmetDataInsert;
        Update: HelmetDataUpdate;
      };
      fall_alerts: {
        Row: FallAlertRow;
        Insert: FallAlertInsert;
        Update: FallAlertUpdate;
      };
      tracking_sessions: {
        Row: TrackingSessionRow;
        Insert: TrackingSessionInsert;
        Update: TrackingSessionUpdate;
      };
    };
  };
}

// Helmet Data Types
export interface HelmetDataRow {
  id: string;
  timestamp: string;
  battery_voltage: number;
  battery_percentage: number;
  remaining_capacity: number;
  solar_current: number;
  temperature: number;
  accel_x: number;
  accel_y: number;
  accel_z: number;
  gyro_x: number;
  gyro_y: number;
  gyro_z: number;
  position_x: number;
  position_y: number;
  direction: Direction;
  heading: number;
  speed: number;
  altitude: number;
  distance_traveled: number;
}

export interface HelmetDataInsert {
  battery_voltage: number;
  battery_percentage: number;
  remaining_capacity: number;
  solar_current: number;
  temperature: number;
  accel_x: number;
  accel_y: number;
  accel_z: number;
  gyro_x: number;
  gyro_y: number;
  gyro_z: number;
  position_x: number;
  position_y: number;
  direction: Direction;
  heading?: number;
  speed: number;
  altitude: number;
  distance_traveled: number;
}

export interface HelmetDataUpdate {
  battery_voltage?: number;
  battery_percentage?: number;
  remaining_capacity?: number;
  solar_current?: number;
  temperature?: number;
  accel_x?: number;
  accel_y?: number;
  accel_z?: number;
  gyro_x?: number;
  gyro_y?: number;
  gyro_z?: number;
  position_x?: number;
  position_y?: number;
  direction?: Direction;
  heading?: number;
  speed?: number;
  altitude?: number;
  distance_traveled?: number;
}

// Fall Alert Types
export interface FallAlertRow {
  id: string;
  timestamp: string;
  position_x: number;
  position_y: number;
  accel_magnitude: number;
  acknowledged: boolean;
}

export interface FallAlertInsert {
  position_x: number;
  position_y: number;
  accel_magnitude: number;
  acknowledged?: boolean;
}

export interface FallAlertUpdate {
  acknowledged?: boolean;
}

// Tracking Session Types
export interface TrackingSessionRow {
  id: string;
  started_at: string;
  reset_count: number;
  last_reset_at: string;
}

export interface TrackingSessionInsert {
  reset_count?: number;
  last_reset_at?: string;
}

export interface TrackingSessionUpdate {
  reset_count?: number;
  last_reset_at?: string;
}

// Cardinal Directions
export type Direction = 'N' | 'NE' | 'E' | 'SE' | 'S' | 'SW' | 'W' | 'NW';

// API Request/Response Types
export interface SensorDataPayload {
  battery_voltage: number;
  battery_percentage: number;
  remaining_capacity: number;
  solar_current: number;
  temperature: number;
  accel: {
    x: number;
    y: number;
    z: number;
  };
  gyro: {
    x: number;
    y: number;
    z: number;
  };
  position: {
    x: number;
    y: number;
  };
  direction: Direction;
  heading?: number;
  speed: number;
  altitude: number;
  distance_traveled: number;
}

export interface FallAlertPayload {
  position_x: number;
  position_y: number;
  accel_magnitude: number;
}

export interface HelmetStatusResponse {
  helmet_data: HelmetDataRow | null;
  unacknowledged_falls: FallAlertRow[];
  reset_requested: boolean;
}

export interface HistoryQueryParams {
  limit?: number;
  offset?: number;
  start_date?: string;
  end_date?: string;
}

// UI Component Props Types
export interface BatteryGaugeProps {
  voltage: number;
  percentage: number;
  capacity: number;
  solarCurrent: number;
}

export interface LocationMapProps {
  currentPosition: { x: number; y: number };
  direction: Direction;
  heading: number;
  history: Array<{ x: number; y: number; timestamp: string }>;
  distanceTraveled?: number;
}

export interface FallAlertProps {
  alerts: FallAlertRow[];
  onAcknowledge: (id: string) => void;
}

export interface SensorReadingsProps {
  temperature: number;
  voltage: number;
  speed: number;
  altitude: number;
  distanceTraveled: number;
}

// Utility Types
export interface Position {
  x: number;
  y: number;
}

export interface HistoricalPoint extends Position {
  timestamp: string;
}

// Constants
export const DIRECTION_LABELS: Record<Direction, string> = {
  N: 'North',
  NE: 'Northeast',
  E: 'East',
  SE: 'Southeast',
  S: 'South',
  SW: 'Southwest',
  W: 'West',
  NW: 'Northwest',
};

export const DIRECTION_ANGLES: Record<Direction, number> = {
  N: 0,
  NE: 45,
  E: 90,
  SE: 135,
  S: 180,
  SW: 225,
  W: 270,
  NW: 315,
};

// Helper Functions
export function getBatteryColor(percentage: number): string {
  if (percentage > 30) return '#10b981'; // green
  if (percentage > 10) return '#f59e0b'; // yellow
  return '#ef4444'; // red
}

export function formatTimestamp(timestamp: string): string {
  return new Date(timestamp).toLocaleString();
}

export function formatSpeed(speed: number): string {
  return `${speed.toFixed(1)} km/h`;
}

export function formatDistance(distance: number): string {
  if (distance < 1000) {
    return `${distance.toFixed(1)} m`;
  }
  return `${(distance / 1000).toFixed(2)} km`;
}

export function formatCapacity(capacity: number): string {
  return `${capacity} mAh`;
}

export function formatVoltage(voltage: number): string {
  return `${voltage.toFixed(2)} V`;
}

export function formatTemperature(temperature: number): string {
  return `${temperature.toFixed(1)}°C`;
}
