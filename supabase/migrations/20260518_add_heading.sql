-- Run in Supabase SQL editor if helmet_data already exists without heading column
ALTER TABLE helmet_data ADD COLUMN IF NOT EXISTS heading FLOAT NOT NULL DEFAULT 0;
