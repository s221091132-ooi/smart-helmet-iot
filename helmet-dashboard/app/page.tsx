// @ts-nocheck
// app/page.tsx - Main dashboard with real-time updates
'use client';

import React, { useState, useEffect, useRef } from 'react';
import BatteryGauge from '@/components/BatteryGauge';
import LocationMap from '@/components/LocationMap';
import FallAlertNotification from '@/components/FallAlertNotification';
import SensorReadings from '@/components/SensorReadings';
import ResetLocationButton from '@/components/ResetLocationButton';
import { supabase } from '@/lib/supabase';
import { HelmetDataRow, FallAlertRow, HistoricalPoint, DIRECTION_LABELS, DIRECTION_ANGLES } from '@/lib/types';

const UPDATE_INTERVAL = 1000; // 1 second
const HISTORY_LIMIT = 50; // Keep last 50 position points

export default function Dashboard() {
  const [helmetData, setHelmetData] = useState<HelmetDataRow | null>(null);
  const [fallAlerts, setFallAlerts] = useState<FallAlertRow[]>([]);
  const [positionHistory, setPositionHistory] = useState<HistoricalPoint[]>([]);
  const [isConnected, setIsConnected] = useState(false);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [showLocationResetBanner, setShowLocationResetBanner] = useState(false);
  const prevResetRequested = useRef(false);

  // Fetch latest status from API
  const fetchStatus = async () => {
    try {
      const response = await fetch('/api/helmet/status');
      
      if (!response.ok) {
        throw new Error('Failed to fetch status');
      }

      const data = await response.json();
      
      if (data.helmet_data) {
        setHelmetData(data.helmet_data);
        setIsConnected(true);
        setLastUpdate(new Date());
        setError(null);

        // Add to position history
        setPositionHistory((prev) => {
          const newPoint: HistoricalPoint = {
            x: data.helmet_data.position_x,
            y: data.helmet_data.position_y,
            timestamp: data.helmet_data.timestamp,
          };

          const updated = [...prev, newPoint];
          
          // Keep only last HISTORY_LIMIT points
          if (updated.length > HISTORY_LIMIT) {
            return updated.slice(-HISTORY_LIMIT);
          }
          
          return updated;
        });
      } else {
        setIsConnected(false);
      }

      setFallAlerts(data.unacknowledged_falls || []);

      // Physical button (GPIO 27) or dashboard reset updated tracking_sessions
      if (data.reset_requested && !prevResetRequested.current) {
        setPositionHistory([]);
        setShowLocationResetBanner(true);
        setTimeout(() => setShowLocationResetBanner(false), 5000);
      }
      prevResetRequested.current = !!data.reset_requested;
    } catch (err) {
      console.error('Error fetching status:', err);
      setError('Failed to connect to server');
      setIsConnected(false);
    }
  };

  // Acknowledge fall alert
  const acknowledgeFallAlert = async (alertId: string) => {
    try {
      const { error } = await supabase
        .from('fall_alerts')
        .update({ acknowledged: true })
        .eq('id', alertId);

      if (error) {
        console.error('Error acknowledging fall alert:', error);
        return;
      }

      // Remove from local state
      setFallAlerts((prev) => prev.filter((alert) => alert.id !== alertId));
    } catch (err) {
      console.error('Error acknowledging fall alert:', err);
    }
  };

  // Reset location tracking
  const resetLocation = async () => {
    try {
      const response = await fetch('/api/helmet/reset-location', {
        method: 'POST',
      });

      if (!response.ok) {
        throw new Error('Failed to reset location');
      }

      // Clear position history
      setPositionHistory([]);
      
      console.log('Location reset successful');
    } catch (err) {
      console.error('Error resetting location:', err);
      throw err;
    }
  };

  // Setup polling for real-time updates
  useEffect(() => {
    // Initial fetch
    fetchStatus();

    // Setup interval
    const interval = setInterval(fetchStatus, UPDATE_INTERVAL);

    // Cleanup
    return () => clearInterval(interval);
  }, []);

  return (
    <main className="min-h-screen bg-gray-100 dark:bg-gray-900">
      {/* Fall Alert Notifications */}
      <FallAlertNotification
        alerts={fallAlerts}
        onAcknowledge={acknowledgeFallAlert}
      />

      {/* Header */}
      <header className="bg-white dark:bg-gray-800 shadow-md">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
          <div className="flex items-center justify-between">
            <div>
              <h1 className="text-3xl font-bold text-gray-900 dark:text-white">
                Smart Helmet Monitoring System
              </h1>
              <p className="mt-1 text-sm text-gray-500 dark:text-gray-400">
                Real-time IoT sensor data and location tracking
              </p>
            </div>

            {/* Connection Status */}
            <div className="flex items-center space-x-3">
              <div
                className={`flex items-center px-3 py-2 rounded-full ${
                  isConnected
                    ? 'bg-green-100 dark:bg-green-900 text-green-800 dark:text-green-200'
                    : 'bg-red-100 dark:bg-red-900 text-red-800 dark:text-red-200'
                }`}
              >
                <div
                  className={`w-2 h-2 rounded-full mr-2 ${
                    isConnected ? 'bg-green-500 animate-pulse' : 'bg-red-500'
                  }`}
                />
                <span className="text-sm font-medium">
                  {isConnected ? 'Connected' : 'Disconnected'}
                </span>
              </div>
              
              {lastUpdate && (
                <div className="text-xs text-gray-500 dark:text-gray-400">
                  Last update: {lastUpdate.toLocaleTimeString()}
                </div>
              )}
            </div>
          </div>

          {/* Error Message */}
          {error && (
            <div className="mt-4 p-3 bg-red-100 dark:bg-red-900 border border-red-400 rounded text-red-700 dark:text-red-200 text-sm">
              {error}
            </div>
          )}

          {showLocationResetBanner && (
            <div className="mt-4 p-3 bg-green-100 dark:bg-green-900 border border-green-400 rounded text-green-800 dark:text-green-200 text-sm flex items-center">
              <span className="font-medium">Location reset successfully! Position (0, 0), altitude 0 m.</span>
            </div>
          )}
        </div>
      </header>

      {/* Main Content */}
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {helmetData ? (
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            {/* Left Column */}
            <div className="space-y-6">
              {/* Battery Gauge */}
              <BatteryGauge
                voltage={helmetData.battery_voltage}
                percentage={helmetData.battery_percentage}
                capacity={helmetData.remaining_capacity}
                solarCurrent={helmetData.solar_current}
              />

              {/* Sensor Readings */}
              <SensorReadings
                temperature={helmetData.temperature}
                voltage={helmetData.battery_voltage}
                speed={helmetData.speed}
                altitude={helmetData.altitude}
                distanceTraveled={helmetData.distance_traveled}
              />
            </div>

            {/* Right Column */}
            <div className="space-y-6">
              {/* Location Map */}
              <LocationMap
                currentPosition={{
                  x: helmetData.position_x,
                  y: helmetData.position_y,
                }}
                direction={helmetData.direction}
                heading={
                  typeof helmetData.heading === 'number'
                    ? helmetData.heading
                    : DIRECTION_ANGLES[helmetData.direction]
                }
                history={positionHistory}
              />

              {/* Movement Stats */}
              <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
                <h2 className="text-xl font-bold mb-4 text-gray-800 dark:text-white">
                  Movement Statistics
                </h2>
                <div className="grid grid-cols-2 gap-4">
                  <div className="p-4 bg-gray-50 dark:bg-gray-700 rounded">
                    <div className="text-sm text-gray-600 dark:text-gray-400">Direction</div>
                    <div className="text-2xl font-bold text-gray-900 dark:text-white">
                      {DIRECTION_LABELS[helmetData.direction]}
                    </div>
                  </div>
                  <div className="p-4 bg-gray-50 dark:bg-gray-700 rounded">
                    <div className="text-sm text-gray-600 dark:text-gray-400">Position</div>
                    <div className="text-lg font-bold text-gray-900 dark:text-white">
                      ({helmetData.position_x.toFixed(1)}, {helmetData.position_y.toFixed(1)})m
                    </div>
                  </div>
                </div>
              </div>

              {/* Reset Location Button */}
              <ResetLocationButton onReset={resetLocation} />
            </div>

            {/* Fall Alert History Table */}
            <div className="lg:col-span-2">
              <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
                <h2 className="text-xl font-bold mb-4 text-gray-800 dark:text-white">
                  Recent Activity
                </h2>
                <div className="grid grid-cols-3 gap-4 text-sm">
                  <div className="p-3 bg-blue-50 dark:bg-blue-900 rounded text-center">
                    <div className="text-xs text-gray-600 dark:text-gray-400">Accelerometer</div>
                    <div className="text-lg font-bold text-gray-900 dark:text-white">
                      X:{helmetData.accel_x} Y:{helmetData.accel_y} Z:{helmetData.accel_z}
                    </div>
                  </div>
                  <div className="p-3 bg-purple-50 dark:bg-purple-900 rounded text-center">
                    <div className="text-xs text-gray-600 dark:text-gray-400">Gyroscope</div>
                    <div className="text-lg font-bold text-gray-900 dark:text-white">
                      X:{helmetData.gyro_x} Y:{helmetData.gyro_y} Z:{helmetData.gyro_z}
                    </div>
                  </div>
                  <div className="p-3 bg-green-50 dark:bg-green-900 rounded text-center">
                    <div className="text-xs text-gray-600 dark:text-gray-400">Active Falls</div>
                    <div className="text-3xl font-bold text-gray-900 dark:text-white">
                      {fallAlerts.length}
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        ) : (
          <div className="flex items-center justify-center h-96">
            <div className="text-center">
              <svg
                className="mx-auto h-12 w-12 text-gray-400 animate-spin"
                fill="none"
                viewBox="0 0 24 24"
              >
                <circle
                  className="opacity-25"
                  cx="12"
                  cy="12"
                  r="10"
                  stroke="currentColor"
                  strokeWidth="4"
                />
                <path
                  className="opacity-75"
                  fill="currentColor"
                  d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
                />
              </svg>
              <p className="mt-4 text-lg text-gray-500 dark:text-gray-400">
                Waiting for helmet data...
              </p>
              <p className="mt-2 text-sm text-gray-400 dark:text-gray-500">
                Make sure the ESP32 is powered on and connected to WiFi
              </p>
            </div>
          </div>
        )}
      </div>

      {/* Footer */}
      <footer className="bg-white dark:bg-gray-800 shadow-md mt-12">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
          <p className="text-center text-sm text-gray-500 dark:text-gray-400">
            Smart Helmet IoT System v1.0 | Updates every {UPDATE_INTERVAL / 1000} second(s)
          </p>
        </div>
      </footer>
    </main>
  );
}
