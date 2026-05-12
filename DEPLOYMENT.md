# Vercel Deployment Guide

This guide explains how to deploy the Smart Helmet dashboard to Vercel.

## Prerequisites

- GitHub account
- Vercel account (free tier is sufficient)
- Supabase project with database schema created

## Step 1: Prepare Repository

1. Initialize Git repository (if not already done):
```bash
cd "/Users/hansongnkk/FYP KAIJIE"
git init
git add .
git commit -m "Initial commit: Smart Helmet IoT System"
```

2. Create GitHub repository:
   - Go to https://github.com/new
   - Create a new repository (e.g., "smart-helmet-iot")
   - Don't initialize with README (we already have one)

3. Push to GitHub:
```bash
git remote add origin https://github.com/YOUR_USERNAME/smart-helmet-iot.git
git branch -M main
git push -u origin main
```

## Step 2: Deploy to Vercel

### Option A: Via Vercel Dashboard (Recommended for beginners)

1. Go to https://vercel.com
2. Click "Sign Up" or "Log In" with GitHub
3. Click "Add New" → "Project"
4. Import your GitHub repository
5. Configure project:
   - **Framework Preset**: Next.js
   - **Root Directory**: `helmet-dashboard`
   - **Build Command**: `npm run build` (auto-detected)
   - **Output Directory**: `.next` (auto-detected)

6. Add Environment Variables:
   - Click "Environment Variables"
   - Add:
     - `NEXT_PUBLIC_SUPABASE_URL` = `your_supabase_project_url`
     - `NEXT_PUBLIC_SUPABASE_ANON_KEY` = `your_supabase_anon_key`

7. Click "Deploy"

8. Wait for deployment to complete (2-3 minutes)

9. Your dashboard will be available at:
   ```
   https://your-project-name.vercel.app
   ```

### Option B: Via Vercel CLI

1. Install Vercel CLI:
```bash
npm install -g vercel
```

2. Login to Vercel:
```bash
vercel login
```

3. Navigate to project:
```bash
cd helmet-dashboard
```

4. Deploy:
```bash
vercel
```

5. Follow the prompts:
   - Set up and deploy? **Y**
   - Which scope? (select your account)
   - Link to existing project? **N**
   - What's your project's name? **smart-helmet-dashboard**
   - In which directory is your code located? **./helmet-dashboard**

6. Add environment variables:
```bash
vercel env add NEXT_PUBLIC_SUPABASE_URL
# Paste your Supabase URL when prompted

vercel env add NEXT_PUBLIC_SUPABASE_ANON_KEY
# Paste your Supabase anon key when prompted
```

7. Deploy to production:
```bash
vercel --prod
```

## Step 3: Configure ESP32

1. Copy your Vercel deployment URL (e.g., `https://smart-helmet-xxxxx.vercel.app`)

2. Update ESP32 configuration:
```cpp
// In arduino/smart_helmet/config.h
#define API_ENDPOINT "https://smart-helmet-xxxxx.vercel.app"
```

3. Re-upload Arduino code to ESP32

## Step 4: Verify Deployment

1. Open your Vercel URL in browser
2. You should see "Waiting for helmet data..."
3. Power on ESP32
4. Within 1-2 seconds, data should appear on dashboard

## Troubleshooting

### Build Fails

**Error**: "Module not found"
- **Fix**: Ensure all dependencies are in `package.json`
- Run `npm install` locally to verify

**Error**: "Environment variable not found"
- **Fix**: Add variables in Vercel dashboard under Settings → Environment Variables

### Dashboard Shows Errors

**Error**: "Failed to fetch status"
- **Fix**: Check API routes are accessible at `/api/helmet/*`
- Verify Supabase credentials are correct

**Error**: CORS errors
- **Fix**: Add CORS headers to API routes if needed
- Vercel should handle this automatically for same-origin requests

### ESP32 Can't Connect

**Error**: "HTTP POST failed"
- **Fix**: Verify API_ENDPOINT URL has no trailing slash
- Check Vercel logs for incoming requests
- Ensure Supabase RLS policies allow inserts

## Continuous Deployment

Once connected to GitHub, Vercel automatically deploys:
- **Production**: When you push to `main` branch
- **Preview**: For every pull request

To update deployment:
```bash
git add .
git commit -m "Update description"
git push
```

Vercel will rebuild and deploy automatically.

## Custom Domain (Optional)

1. Go to Vercel dashboard → Your project → Settings → Domains
2. Click "Add Domain"
3. Enter your domain name (e.g., `smart-helmet.yourdomain.com`)
4. Follow DNS configuration instructions
5. Wait for DNS propagation (5-60 minutes)

## Monitoring

### View Logs
1. Go to Vercel dashboard
2. Select your project
3. Click "Logs" tab
4. Filter by API route or time range

### Analytics
1. Enable Analytics in project settings
2. View real-time metrics:
   - Request count
   - Error rate
   - Response times
   - Geographic distribution

## Cost Considerations

### Vercel Free Tier Limits:
- 100 GB bandwidth/month
- Unlimited deployments
- 100 GB-hours serverless function execution

### Estimated Usage:
- 1 ESP32 sending data every 1 second
- ~86,400 requests/day
- ~2.6M requests/month
- Should stay within free tier

If you exceed limits, consider:
- Increasing data send interval (e.g., 5 seconds instead of 1)
- Upgrading to Vercel Pro ($20/month)

## Security Best Practices

1. **Never commit** `.env.local` to Git
2. Use environment variables for all secrets
3. Enable Supabase RLS policies
4. Consider adding API authentication for production
5. Monitor Vercel logs for suspicious activity

## Backup and Rollback

### Backup
Vercel keeps all deployments. No manual backup needed.

### Rollback
1. Go to Vercel dashboard → Your project → Deployments
2. Find working deployment
3. Click "..." → "Promote to Production"

## Next Steps

After successful deployment:
1. Test all features with ESP32
2. Share dashboard URL with team
3. Set up monitoring/alerting
4. Configure custom domain if desired
5. Enable analytics for insights

## Support

- Vercel Documentation: https://vercel.com/docs
- Vercel Discord: https://vercel.com/discord
- GitHub Issues: Create issue in your repository
