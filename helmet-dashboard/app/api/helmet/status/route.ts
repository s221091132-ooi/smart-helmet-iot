// app/api/helmet/status/route.ts - Get current helmet status
import { NextResponse } from 'next/server';
import { supabase } from '@/lib/supabase';
import { HelmetStatusResponse } from '@/lib/types';

export async function GET() {
  try {
    // Get latest helmet data
    const { data: helmetData, error: helmetError } = await supabase
      .from('helmet_data')
      .select('*')
      .order('timestamp', { ascending: false })
      .limit(1)
      .single();

    if (helmetError && helmetError.code !== 'PGRST116') {
      console.error('Error fetching helmet data:', helmetError);
      return NextResponse.json(
        { error: 'Database error', details: helmetError.message },
        { status: 500 }
      );
    }

    // Get unacknowledged fall alerts
    const { data: fallAlerts, error: fallError } = await supabase
      .from('fall_alerts')
      .select('*')
      .eq('acknowledged', false)
      .order('timestamp', { ascending: false });

    if (fallError) {
      console.error('Error fetching fall alerts:', fallError);
      return NextResponse.json(
        { error: 'Database error', details: fallError.message },
        { status: 500 }
      );
    }

    // Check if location reset was recently requested
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
      
      // If reset was requested in the last 10 seconds
      if (timeDiff < 10000) {
        resetRequested = true;
      }
    }

    const response: HelmetStatusResponse = {
      helmet_data: helmetData || null,
      unacknowledged_falls: fallAlerts || [],
      reset_requested: resetRequested,
    };

    return NextResponse.json(response);
  } catch (error) {
    console.error('API error:', error);
    return NextResponse.json(
      { error: 'Internal server error', details: String(error) },
      { status: 500 }
    );
  }
}
