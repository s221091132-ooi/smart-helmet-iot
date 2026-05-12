// components/FallAlertNotification.tsx - Fall alert notification with acknowledge button
'use client';

import React from 'react';
import { FallAlertProps, formatTimestamp } from '@/lib/types';

export default function FallAlertNotification({
  alerts,
  onAcknowledge,
}: FallAlertProps) {
  if (alerts.length === 0) {
    return null;
  }

  return (
    <div className="fixed top-4 right-4 z-50 max-w-md space-y-2">
      {alerts.map((alert) => (
        <div
          key={alert.id}
          className="bg-red-500 text-white rounded-lg shadow-2xl p-4 animate-pulse"
          role="alert"
        >
          <div className="flex items-start">
            {/* Alert Icon */}
            <div className="flex-shrink-0">
              <svg
                className="w-6 h-6"
                fill="currentColor"
                viewBox="0 0 20 20"
              >
                <path
                  fillRule="evenodd"
                  d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z"
                  clipRule="evenodd"
                />
              </svg>
            </div>

            {/* Alert Content */}
            <div className="ml-3 flex-1">
              <h3 className="text-lg font-bold">FALL DETECTED!</h3>
              <p className="mt-1 text-sm">
                A fall has been detected at position ({alert.position_x.toFixed(1)}, {alert.position_y.toFixed(1)})
              </p>
              <p className="mt-1 text-xs opacity-90">
                Impact: {alert.accel_magnitude.toFixed(1)} m/s²
              </p>
              <p className="mt-1 text-xs opacity-90">
                Time: {formatTimestamp(alert.timestamp)}
              </p>

              {/* Acknowledge Button */}
              <button
                onClick={() => onAcknowledge(alert.id)}
                className="mt-3 w-full bg-white text-red-600 font-semibold py-2 px-4 rounded hover:bg-gray-100 transition-colors"
              >
                Acknowledge Alert
              </button>
            </div>

            {/* Close Button */}
            <button
              onClick={() => onAcknowledge(alert.id)}
              className="flex-shrink-0 ml-4 text-white hover:text-gray-200 transition-colors"
            >
              <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
                <path
                  fillRule="evenodd"
                  d="M4.293 4.293a1 1 0 011.414 0L10 8.586l4.293-4.293a1 1 0 111.414 1.414L11.414 10l4.293 4.293a1 1 0 01-1.414 1.414L10 11.414l-4.293 4.293a1 1 0 01-1.414-1.414L8.586 10 4.293 5.707a1 1 0 010-1.414z"
                  clipRule="evenodd"
                />
              </svg>
            </button>
          </div>
        </div>
      ))}
    </div>
  );
}
