// components/LocationMap.tsx - 2D location tracking map with grid and dot trail
'use client';

import React, { useEffect, useRef } from 'react';
import { LocationMapProps, DIRECTION_ANGLES } from '@/lib/types';

export default function LocationMap({
  currentPosition,
  direction,
  heading,
  history,
}: LocationMapProps) {
  const compassAngle = heading ?? DIRECTION_ANGLES[direction];
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas size
    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.clearRect(0, 0, width, height);

    // Calculate scale and center
    const centerX = width / 2;
    const centerY = height / 2;
    const scale = 5; // 5 pixels per meter

    // Draw grid
    ctx.strokeStyle = '#e5e7eb';
    ctx.lineWidth = 1;

    // Vertical lines (every 10 meters)
    for (let x = -50; x <= 50; x += 10) {
      const canvasX = centerX + x * scale;
      ctx.beginPath();
      ctx.moveTo(canvasX, 0);
      ctx.lineTo(canvasX, height);
      ctx.stroke();
    }

    // Horizontal lines (every 10 meters)
    for (let y = -50; y <= 50; y += 10) {
      const canvasY = centerY - y * scale;
      ctx.beginPath();
      ctx.moveTo(0, canvasY);
      ctx.lineTo(width, canvasY);
      ctx.stroke();
    }

    // Draw axes
    ctx.strokeStyle = '#9ca3af';
    ctx.lineWidth = 2;

    // X-axis
    ctx.beginPath();
    ctx.moveTo(0, centerY);
    ctx.lineTo(width, centerY);
    ctx.stroke();

    // Y-axis
    ctx.beginPath();
    ctx.moveTo(centerX, 0);
    ctx.lineTo(centerX, height);
    ctx.stroke();

    // Draw origin marker
    ctx.fillStyle = '#ef4444';
    ctx.beginPath();
    ctx.arc(centerX, centerY, 6, 0, 2 * Math.PI);
    ctx.fill();
    
    // Origin label
    ctx.fillStyle = '#374151';
    ctx.font = '12px sans-serif';
    ctx.fillText('(0, 0)', centerX + 10, centerY - 10);

    // Draw historical positions as small dots
    ctx.fillStyle = '#6366f1';
    history.forEach((point) => {
      const x = centerX + point.x * scale;
      const y = centerY - point.y * scale;
      
      ctx.beginPath();
      ctx.arc(x, y, 3, 0, 2 * Math.PI);
      ctx.fill();
    });

    // Draw current position
    const currentX = centerX + currentPosition.x * scale;
    const currentY = centerY - currentPosition.y * scale;

    // Current position circle
    ctx.fillStyle = '#10b981';
    ctx.beginPath();
    ctx.arc(currentX, currentY, 8, 0, 2 * Math.PI);
    ctx.fill();

    // Outer ring for pulse effect
    ctx.strokeStyle = '#10b981';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(currentX, currentY, 12, 0, 2 * Math.PI);
    ctx.stroke();

    // Draw direction arrow
    const angle = (compassAngle * Math.PI) / 180;
    const arrowLength = 20;
    const arrowEndX = currentX + Math.sin(angle) * arrowLength;
    const arrowEndY = currentY - Math.cos(angle) * arrowLength;

    ctx.strokeStyle = '#10b981';
    ctx.lineWidth = 3;
    ctx.lineCap = 'round';
    
    // Arrow line
    ctx.beginPath();
    ctx.moveTo(currentX, currentY);
    ctx.lineTo(arrowEndX, arrowEndY);
    ctx.stroke();

    // Arrow head
    const arrowHeadSize = 8;
    const angle1 = angle - Math.PI / 6;
    const angle2 = angle + Math.PI / 6;
    
    ctx.beginPath();
    ctx.moveTo(arrowEndX, arrowEndY);
    ctx.lineTo(
      arrowEndX - Math.sin(angle1) * arrowHeadSize,
      arrowEndY + Math.cos(angle1) * arrowHeadSize
    );
    ctx.moveTo(arrowEndX, arrowEndY);
    ctx.lineTo(
      arrowEndX - Math.sin(angle2) * arrowHeadSize,
      arrowEndY + Math.cos(angle2) * arrowHeadSize
    );
    ctx.stroke();

    // Position label
    ctx.fillStyle = '#374151';
    ctx.font = 'bold 12px sans-serif';
    ctx.fillText(
      `(${currentPosition.x.toFixed(1)}, ${currentPosition.y.toFixed(1)})`,
      currentX + 15,
      currentY - 15
    );

  }, [currentPosition, direction, heading, history, compassAngle]);

  return (
    <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
      <h2 className="text-xl font-bold mb-4 text-gray-800 dark:text-white">
        Location Tracking
      </h2>

      <div className="relative">
        <canvas
          ref={canvasRef}
          width={500}
          height={500}
          className="border border-gray-300 dark:border-gray-600 rounded bg-white dark:bg-gray-900"
        />

        {/* Legend */}
        <div className="mt-4 flex items-center justify-between text-sm text-gray-600 dark:text-gray-400">
          <div className="flex items-center space-x-4">
            <div className="flex items-center">
              <div className="w-3 h-3 rounded-full bg-red-500 mr-2"></div>
              <span>Origin</span>
            </div>
            <div className="flex items-center">
              <div className="w-3 h-3 rounded-full bg-indigo-500 mr-2"></div>
              <span>Path</span>
            </div>
            <div className="flex items-center">
              <div className="w-3 h-3 rounded-full bg-green-500 mr-2"></div>
              <span>Current</span>
            </div>
          </div>
          <div className="text-xs">
            Scale: 1 grid = 10m
          </div>
        </div>

        {/* Compass */}
        <div className="absolute top-4 right-4 bg-white dark:bg-gray-800 border border-gray-300 dark:border-gray-600 rounded-full p-3 shadow-lg">
          <div className="relative w-16 h-16">
            <svg viewBox="0 0 64 64" className="w-full h-full">
              {/* Compass circle */}
              <circle cx="32" cy="32" r="30" fill="none" stroke="#9ca3af" strokeWidth="2" />
              
              {/* Cardinal directions */}
              <text x="32" y="12" textAnchor="middle" fontSize="10" fill="#374151" fontWeight="bold">N</text>
              <text x="52" y="36" textAnchor="middle" fontSize="10" fill="#374151">E</text>
              <text x="32" y="56" textAnchor="middle" fontSize="10" fill="#374151">S</text>
              <text x="12" y="36" textAnchor="middle" fontSize="10" fill="#374151">W</text>
              
              {/* Direction indicator */}
              <g transform={`rotate(${compassAngle} 32 32)`}>
                <polygon points="32,18 28,28 32,26 36,28" fill="#10b981" />
                <polygon points="32,46 28,36 32,38 36,36" fill="#6b7280" />
              </g>
            </svg>
          </div>
        </div>
      </div>
    </div>
  );
}
