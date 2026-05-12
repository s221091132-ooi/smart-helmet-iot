// components/SensorReadings.tsx - Display sensor readings
'use client';

import React from 'react';
import { SensorReadingsProps, formatTemperature, formatSpeed, formatDistance, DIRECTION_LABELS } from '@/lib/types';

export default function SensorReadings({
  temperature,
  voltage,
  speed,
  altitude,
  distanceTraveled,
}: SensorReadingsProps) {
  return (
    <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
      <h2 className="text-xl font-bold mb-4 text-gray-800 dark:text-white">
        Sensor Readings
      </h2>

      <div className="space-y-4">
        {/* Temperature */}
        <div className="flex items-center justify-between p-4 bg-gradient-to-r from-orange-50 to-red-50 dark:from-orange-900 dark:to-red-900 rounded-lg">
          <div className="flex items-center">
            <svg className="w-8 h-8 text-orange-500 mr-3" fill="currentColor" viewBox="0 0 20 20">
              <path fillRule="evenodd" d="M10 2a1 1 0 011 1v6.586l2.707 2.707a1 1 0 01-1.414 1.414L9 10.414V3a1 1 0 011-1z" clipRule="evenodd" />
              <path d="M6 6a4 4 0 108 0 4 4 0 00-8 0z" />
            </svg>
            <div>
              <div className="text-sm text-gray-600 dark:text-gray-400">Temperature</div>
              <div className="text-2xl font-bold text-gray-900 dark:text-white">
                {formatTemperature(temperature)}
              </div>
            </div>
          </div>
        </div>

        {/* Speed */}
        <div className="flex items-center justify-between p-4 bg-gradient-to-r from-blue-50 to-cyan-50 dark:from-blue-900 dark:to-cyan-900 rounded-lg">
          <div className="flex items-center">
            <svg className="w-8 h-8 text-blue-500 mr-3" fill="currentColor" viewBox="0 0 20 20">
              <path d="M10 2a6 6 0 00-6 6v3.586l-.707.707A1 1 0 004 14h12a1 1 0 00.707-1.707L16 11.586V8a6 6 0 00-6-6zM10 18a3 3 0 01-3-3h6a3 3 0 01-3 3z" />
            </svg>
            <div>
              <div className="text-sm text-gray-600 dark:text-gray-400">Current Speed</div>
              <div className="text-2xl font-bold text-gray-900 dark:text-white">
                {formatSpeed(speed)}
              </div>
            </div>
          </div>
        </div>

        {/* Distance Traveled */}
        <div className="flex items-center justify-between p-4 bg-gradient-to-r from-green-50 to-emerald-50 dark:from-green-900 dark:to-emerald-900 rounded-lg">
          <div className="flex items-center">
            <svg className="w-8 h-8 text-green-500 mr-3" fill="currentColor" viewBox="0 0 20 20">
              <path fillRule="evenodd" d="M5.05 4.05a7 7 0 119.9 9.9L10 18.9l-4.95-4.95a7 7 0 010-9.9zM10 11a2 2 0 100-4 2 2 0 000 4z" clipRule="evenodd" />
            </svg>
            <div>
              <div className="text-sm text-gray-600 dark:text-gray-400">Distance Traveled</div>
              <div className="text-2xl font-bold text-gray-900 dark:text-white">
                {formatDistance(distanceTraveled)}
              </div>
            </div>
          </div>
        </div>

        {/* Altitude */}
        <div className="flex items-center justify-between p-4 bg-gradient-to-r from-purple-50 to-pink-50 dark:from-purple-900 dark:to-pink-900 rounded-lg">
          <div className="flex items-center">
            <svg className="w-8 h-8 text-purple-500 mr-3" fill="currentColor" viewBox="0 0 20 20">
              <path fillRule="evenodd" d="M3.172 5.172a4 4 0 015.656 0L10 6.343l1.172-1.171a4 4 0 115.656 5.656L10 17.657l-6.828-6.829a4 4 0 010-5.656z" clipRule="evenodd" />
            </svg>
            <div>
              <div className="text-sm text-gray-600 dark:text-gray-400">Altitude</div>
              <div className="text-2xl font-bold text-gray-900 dark:text-white">
                {altitude.toFixed(1)} m
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
