# Claude Clone - Full Stack Application

A complete uncensored clone of Claude, built with:
- **Backend**: Node.js + Express + Ollama LLM
- **Mobile**: React Native (Android/iOS)
- **Database**: SQLite (local)
- **LLM**: Ollama (local, no censorship)

## Features

✅ **Chat Interface** - Real-time AI conversation  
✅ **Conversation History** - Save and manage chats  
✅ **Multiple Models** - Support for any Ollama model  
✅ **Uncensored** - Running locally, fully under your control  
✅ **Free & Open** - No API keys, no subscriptions  
✅ **Cross-platform** - Android and iOS support  

## Prerequisites

- **Node.js** 18+ (for backend)
- **Ollama** installed and running (download from [ollama.ai](https://ollama.ai))
- **Android SDK** (for APK build)
- **npm** or **yarn**

## Installation & Setup

### 1. Start Ollama Service

```bash
# Download and install Ollama
# Then start it:
ollama serve

# In another terminal, pull a model (if not already done)
ollama pull mistral
# Or try others:
# ollama pull neural-chat
# ollama pull dolphin-mixtral
```

### 2. Setup Backend

```bash
cd backend

# Install dependencies
npm install

# Create .env file
cp .env.example .env

# Build
npm run build

# Start development server
npm run dev

# Or start production server
npm run start
```

Backend will run on `http://localhost:3000`

### 3. Setup Mobile App

#### For Android APK:

```bash
cd mobile

# Install dependencies
npm install

# Prepare environment
cp .env.example .env

# Install Android build tools
npx react-native-cli@latest
cd android

# Build APK (debug)
./gradlew assembleDebug

# Build APK (release)
./gradlew assembleRelease

# APK will be at:
# Debug: ./app/build/outputs/apk/debug/app-debug.apk
# Release: ./app/build/outputs/apk/release/app-release.apk
```

#### For iOS:

```bash
cd mobile

# Install dependencies
npm install

# Install pods
cd ios && pod install && cd ..

# Run on simulator
npm run ios

# Or build for production via Xcode
```

## Usage

### Backend API Endpoints

```bash
# Create conversation
POST /api/chat/conversations
{ "title": "My Chat" }

# Send message
POST /api/chat/conversations/{id}/messages
{ "content": "Hello", "model": "mistral" }

# Stream response
POST /api/chat/conversations/{id}/messages/stream

# Get conversations
GET /api/chat/conversations

# List models
GET /api/models

# Health check
GET /health
GET /api/llm/status
```

### Mobile App

1. Install APK on Android device
2. Open Settings and set API URL to your backend
3. Ensure Ollama is running with models available
4. Start chatting!

## Configuration

### Environment Variables

**Backend** (`.env`):
```
NODE_ENV=development
PORT=3000
OLLAMA_URL=http://localhost:11434
LLM_MODEL=mistral
```

**Mobile** (`.env`):
```
REACT_APP_API_URL=http://localhost:3000
```

### Available Ollama Models

```bash
ollama pull mistral           # Fast, good quality
ollama pull neural-chat       # Optimized for chat
ollama pull dolphin-mixtral   # Very capable
ollama pull orca2             # Another option
ollama pull llama2            # Meta's model
```

## Project Structure

```
claude-clone/
├── backend/
│   ├── src/
│   │   ├── index.ts          # Express app
│   │   ├── services/
│   │   │   └── ollama.ts     # LLM integration
│   │   ├── routes/
│   │   │   ├── chat.ts
│   │   │   ├── models.ts
│   │   │   └── files.ts
│   │   └── db/
│   │       └── init.ts       # SQLite setup
│   ├── package.json
│   └── tsconfig.json
│
├── mobile/
│   ├── App.tsx               # Root component
│   ├── screens/
│   │   ├── ChatScreen.tsx
│   │   ├── ConversationsScreen.tsx
│   │   └── SettingsScreen.tsx
│   ├── services/
│   │   └── api.ts            # API client
│   ├── package.json
│   └── app.json
│
└── shared/
    └── types.ts              # Shared types
```

## Building Release APK

### Step-by-step:

1. **Prepare keystore** (first time only):
   ```bash
   keytool -genkey -v -keystore my-release-key.keystore -keyalg RSA -keysize 2048 -validity 10000 -alias my-key-alias
   ```

2. **Configure gradle properties**:
   ```
   # android/gradle.properties
   MYAPP_RELEASE_STORE_FILE=my-release-key.keystore
   MYAPP_RELEASE_KEY_ALIAS=my-key-alias
   MYAPP_RELEASE_STORE_PASSWORD=xxxxx
   MYAPP_RELEASE_KEY_PASSWORD=xxxxx
   ```

3. **Build release APK**:
   ```bash
   cd mobile/android
   ./gradlew assembleRelease
   ```

4. **Find APK**:
   ```
   mobile/android/app/build/outputs/apk/release/app-release.apk
   ```

## Deployment

### Local Deployment (Recommended)

Run both backend and Ollama on the same machine:
```bash
# Terminal 1 - Ollama
ollama serve

# Terminal 2 - Backend
cd claude-clone/backend && npm start

# Terminal 3 - Mobile
# Install APK and set API URL to backend address
```

### Remote Deployment

To deploy backend to server:
1. Setup Node.js on server
2. Setup Ollama on server (or use remote URL)
3. Deploy backend (npm install && npm start)
4. Update mobile API URL to server address

## Troubleshooting

### APK Installation Issues

```bash
# Enable unknown sources on Android
# Settings > Security > Unknown Sources (enable)

# Install APK manually
adb install path/to/app.apk

# Check logs
adb logcat | grep Claude
```

### Connection Issues

- Ensure Ollama is running: `curl http://localhost:11434/api/tags`
- Check backend is running: `curl http://localhost:3000/health`
- Verify API URL in mobile settings matches backend address

### LLM Not Responding

```bash
# Check available models
curl http://localhost:11434/api/tags

# Pull a new model
ollama pull mistral

# Check Ollama logs
# Ollama runs on http://localhost:11434
```

## Performance Notes

- First LLM response may take 5-30 seconds (model loading)
- Subsequent responses are faster
- Use smaller models for mobile (mistral, neural-chat, orca2)
- Larger models (dolphin-mixtral) require more RAM

## License

MIT - Free to use and modify

## Notes

- **No Censorship**: Running on your local machine, you have full control
- **Private**: All data stays on your device/server
- **Free**: No API costs, no subscriptions
- **Open Source**: Fully customizable

## Support

Check the logs:
```bash
# Backend logs
tail -f ~/.claude-clone/backend.log

# Mobile logs
adb logcat
```

## Future Improvements

- [ ] Image support
- [ ] File upload/analysis
- [ ] Voice input/output
- [ ] Web interface
- [ ] Cloud sync (optional)
- [ ] Plugin system
- [ ] Custom models training
