// app/api/helmet/fall/route.ts - Receive fall detection alerts
import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/supabase';
import { FallAlertPayload } from '@/lib/types';

export async function POST(request: NextRequest) {
  try {
    // Parse request body
    const payload: FallAlertPayload = await request.json();

    // Validate required fields
    if (
      typeof payload.position_x !== 'number' ||
      typeof payload.position_y !== 'number' ||
      typeof payload.accel_magnitude !== 'number'
    ) {
      return NextResponse.json(
        { error: 'Invalid payload: missing required fields' },
        { status: 400 }
      );
    }

    // Insert fall alert into Supabase
    const { data, error } = await supabase
      .from('fall_alerts')
      .insert({
        position_x: payload.position_x,
        position_y: payload.position_y,
        accel_magnitude: payload.accel_magnitude,
        acknowledged: false,
      })
      .select();

    if (error) {
      console.error('Supabase insert error:', error);
      return NextResponse.json(
        { error: 'Database error', details: error.message },
        { status: 500 }
      );
    }

    console.log('Fall alert recorded:', data[0]);

    // TODO: Add notification service integration here (email, SMS, push notification)
    // Example: await sendNotification('Fall detected!', data[0]);

    return NextResponse.json(
      {
        success: true,
        message: 'Fall alert received',
        alert: data[0],
      },
      { status: 201 }
    );
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
    { message: 'POST fall alert data to this endpoint' },
    { status: 200 }
  );
}
