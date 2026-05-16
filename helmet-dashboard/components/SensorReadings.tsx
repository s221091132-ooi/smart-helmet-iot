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
  const isHighTemp = temperature >= 35;  // Changed from > to >= (include 35.0°C)

  return (
    <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
      <h2 className="text-xl font-bold mb-4 text-gray-800 dark:text-white">
        Sensor Readings
      </h2>

      <div className="space-y-4">
        {/* High Temperature Warning Banner */}
        {isHighTemp && (
          <div className="p-4 bg-red-100 dark:bg-red-900 border-2 border-red-500 rounded-lg animate-pulse">
            <div className="flex items-center">
              <svg className="w-6 h-6 text-red-600 dark:text-red-300 mr-3" fill="currentColor" viewBox="0 0 20 20">
                <path fillRule="evenodd" d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z" clipRule="evenodd" />
              </svg>
              <div>
                <div className="text-sm font-bold text-red-800 dark:text-red-200">
                  ⚠️ HIGH TEMPERATURE ALERT!
                </div>
                <div className="text-xs text-red-700 dark:text-red-300 mt-1">
                  Temperature exceeds safe threshold (35°C). Please cool down immediately!
                </div>
              </div>
            </div>
          </div>
        )}

        {/* Temperature */}
        <div className={`flex items-center justify-between p-4 rounded-lg ${
          isHighTemp 
            ? 'bg-gradient-to-r from-red-100 to-red-200 dark:from-red-900 dark:to-red-800 border-2 border-red-500' 
            : 'bg-gradient-to-r from-orange-50 to-red-50 dark:from-orange-900 dark:to-red-900'
        }`}>
          <div className="flex items-center">
            <svg className={`w-8 h-8 mr-3 ${isHighTemp ? 'text-red-600 dark:text-red-300' : 'text-orange-500'}`} fill="currentColor" viewBox="0 0 20 20">
              <path fillRule="evenodd" d="M10 2a1 1 0 011 1v6.586l2.707 2.707a1 1 0 01-1.414 1.414L9 10.414V3a1 1 0 011-1z" clipRule="evenodd" />
              <path d="M6 6a4 4 0 108 0 4 4 0 00-8 0z" />
            </svg>
            <div>
              <div className={`text-sm ${isHighTemp ? 'text-red-700 dark:text-red-200 font-bold' : 'text-gray-600 dark:text-gray-400'}`}>
                Temperature {isHighTemp && '⚠️'}
              </div>
              <div className={`text-2xl font-bold ${isHighTemp ? 'text-red-800 dark:text-red-100' : 'text-gray-900 dark:text-white'}`}>
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
