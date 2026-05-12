// app/api/helmet/reset-location/route.ts - Reset location tracking origin
import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/supabase';

export async function POST(request: NextRequest) {
  try {
    // Get current tracking session
    const { data: currentSession, error: fetchError } = await supabase
      .from('tracking_sessions')
      .select('*')
      .order('started_at', { ascending: false })
      .limit(1)
      .single();

    if (fetchError && fetchError.code !== 'PGRST116') {
      console.error('Error fetching tracking session:', fetchError);
      return NextResponse.json(
        { error: 'Database error', details: fetchError.message },
        { status: 500 }
      );
    }

    const now = new Date().toISOString();

    if (currentSession) {
      // Update existing session
      const { data, error } = await supabase
        .from('tracking_sessions')
        .update({
          reset_count: currentSession.reset_count + 1,
          last_reset_at: now,
        } as any)
        .eq('id', currentSession.id)
        .select();

      if (error) {
        console.error('Error updating tracking session:', error);
        return NextResponse.json(
          { error: 'Database error', details: error.message },
          { status: 500 }
        );
      }

      console.log('Location reset:', data[0]);

      return NextResponse.json({
        success: true,
        message: 'Location reset requested',
        session: data[0],
      });
    } else {
      // Create new session
      const { data, error } = await supabase
        .from('tracking_sessions')
        .insert([{
          reset_count: 1,
          last_reset_at: now,
        }] as any)
        .select();

      if (error) {
        console.error('Error creating tracking session:', error);
        return NextResponse.json(
          { error: 'Database error', details: error.message },
          { status: 500 }
        );
      }

      console.log('New tracking session created:', data[0]);

      return NextResponse.json({
        success: true,
        message: 'Location reset requested',
        session: data[0],
      });
    }
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
    { message: 'POST to this endpoint to reset location tracking' },
    { status: 200 }
  );
}
