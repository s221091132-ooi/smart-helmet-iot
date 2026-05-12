// app/api/helmet/data/route.ts - Receive sensor data from ESP32
import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/supabase';
import { SensorDataPayload } from '@/lib/types';

export async function POST(request: NextRequest) {
  try {
    // Parse request body
    const payload: SensorDataPayload = await request.json();

    // Validate required fields
    if (
      typeof payload.battery_voltage !== 'number' ||
      typeof payload.battery_percentage !== 'number' ||
      typeof payload.remaining_capacity !== 'number' ||
      typeof payload.solar_current !== 'number' ||
      typeof payload.temperature !== 'number' ||
      !payload.accel ||
      !payload.gyro ||
      !payload.position ||
      !payload.direction
    ) {
      return NextResponse.json(
        { error: 'Invalid payload: missing required fields' },
        { status: 400 }
      );
    }

    // Insert data into Supabase
    const { data, error } = await supabase
      .from('helmet_data')
      .insert([{
        battery_voltage: payload.battery_voltage,
        battery_percentage: payload.battery_percentage,
        remaining_capacity: payload.remaining_capacity,
        solar_current: payload.solar_current,
        temperature: payload.temperature,
        accel_x: payload.accel.x,
        accel_y: payload.accel.y,
        accel_z: payload.accel.z,
        gyro_x: payload.gyro.x,
        gyro_y: payload.gyro.y,
        gyro_z: payload.gyro.z,
        position_x: payload.position.x,
        position_y: payload.position.y,
        direction: payload.direction,
        speed: payload.speed,
        altitude: payload.altitude,
        distance_traveled: payload.distance_traveled,
      }] as any)
      .select();

    if (error) {
      console.error('Supabase insert error:', error);
      return NextResponse.json(
        { error: 'Database error', details: error.message },
        { status: 500 }
      );
    }

    // Check if location reset was requested
    const { data: sessionData, error: sessionError } = await supabase
      .from('tracking_sessions')
      .select('last_reset_at')
      .order('last_reset_at', { ascending: false })
      .limit(1)
      .single();

    let resetRequested = false;
    if (!sessionError && sessionData) {
      const lastReset = new Date(sessionData.last_reset_at);
      const now = new Date();
      const timeDiff = now.getTime() - lastReset.getTime();
      
      // If reset was requested in the last 5 seconds, signal ESP32
      if (timeDiff < 5000) {
        resetRequested = true;
      }
    }

    return NextResponse.json({
      success: true,
      message: 'Data received successfully',
      reset_location: resetRequested,
      data: data[0],
    });
  } catch (error) {
    console.error('API error:', error);
    return NextResponse.json(
      { error: 'Internal server error', details: String(error) },
      { status: 500 }
    );
  }
}

export async function GET() {
  return NextResponse.json(
    { message: 'POST sensor data to this endpoint' },
    { status: 200 }
  );
}
