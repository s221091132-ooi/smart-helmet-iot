// components/BatteryGauge.tsx - Battery level display with circular progress
'use client';

import React from 'react';
import { BatteryGaugeProps, getBatteryColor, formatVoltage, formatCapacity } from '@/lib/types';

export default function BatteryGauge({
  voltage,
  percentage,
  capacity,
  solarCurrent,
}: BatteryGaugeProps) {
  const color = getBatteryColor(percentage);
  const circumference = 2 * Math.PI * 70; // radius = 70
  const strokeDashoffset = circumference - (percentage / 100) * circumference;
  const isCritical = percentage < 10;

  return (
    <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
      <h2 className="text-xl font-bold mb-4 text-gray-800 dark:text-white">
        Battery Status
      </h2>
      
      <div className="flex flex-col items-center">
        {/* Circular Progress */}
        <div className="relative w-48 h-48">
          <svg className="transform -rotate-90 w-48 h-48">
            {/* Background circle */}
            <circle
              cx="96"
              cy="96"
              r="70"
              stroke="#e5e7eb"
              strokeWidth="12"
              fill="none"
            />
            {/* Progress circle */}
            <circle
              cx="96"
              cy="96"
              r="70"
              stroke={color}
              strokeWidth="12"
              fill="none"
              strokeDasharray={circumference}
              strokeDashoffset={strokeDashoffset}
              strokeLinecap="round"
              className={`transition-all duration-500 ${
                isCritical ? 'animate-pulse' : ''
              }`}
            />
          </svg>
          
          {/* Percentage Text */}
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="text-center">
              <div className={`text-4xl font-bold ${isCritical ? 'text-red-500 animate-pulse' : ''}`}
                   style={{ color: isCritical ? '#ef4444' : color }}>
                {percentage}%
              </div>
              <div className="text-sm text-gray-500 dark:text-gray-400 mt-1">
                Battery
              </div>
            </div>
          </div>
        </div>

        {/* Battery Details */}
        <div className="mt-6 w-full space-y-3">
          {/* Voltage */}
          <div className="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700 rounded">
            <span className="text-sm font-medium text-gray-600 dark:text-gray-300">
              Voltage
            </span>
            <span className="text-sm font-bold text-gray-900 dark:text-white">
              {formatVoltage(voltage)}
            </span>
          </div>

          {/* Remaining Capacity */}
          <div className="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700 rounded">
            <span className="text-sm font-medium text-gray-600 dark:text-gray-300">
              Capacity
            </span>
            <span className="text-sm font-bold text-gray-900 dark:text-white">
              {formatCapacity(capacity)}
            </span>
          </div>

          {/* Solar Charging */}
          <div className="flex justify-between items-center p-3 bg-gray-50 dark:bg-gray-700 rounded">
            <span className="text-sm font-medium text-gray-600 dark:text-gray-300">
              Solar Charge
            </span>
            <span className="text-sm font-bold text-gray-900 dark:text-white">
              {solarCurrent > 0 ? (
                <span className="text-green-500 flex items-center">
                  <svg className="w-4 h-4 mr-1" fill="currentColor" viewBox="0 0 20 20">
                    <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-8.707l-3-3a1 1 0 00-1.414 1.414L10.586 9H7a1 1 0 100 2h3.586l-1.293 1.293a1 1 0 101.414 1.414l3-3a1 1 0 000-1.414z" clipRule="evenodd" />
                  </svg>
                  {solarCurrent.toFixed(2)} A
                </span>
              ) : (
                <span className="text-gray-400">Not charging</span>
              )}
            </span>
          </div>
        </div>

        {/* Critical Warning */}
        {isCritical && (
          <div className="mt-4 w-full p-3 bg-red-100 dark:bg-red-900 border border-red-400 rounded flex items-center">
            <svg className="w-5 h-5 text-red-500 dark:text-red-300 mr-2" fill="currentColor" viewBox="0 0 20 20">
              <path fillRule="evenodd" d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z" clipRule="evenodd" />
            </svg>
            <span className="text-sm font-medium text-red-700 dark:text-red-300">
              Battery critically low! Please charge soon.
            </span>
          </div>
        )}
      </div>
    </div>
  );
}
