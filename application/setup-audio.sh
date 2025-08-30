#!/bin/bash

echo "🎵 Setting up Audio Features for Stage LED Dashboard"
echo "=================================================="

# Check if we're in the right directory
if [ ! -f "package.json" ]; then
    echo "❌ Error: Please run this script from the application directory"
    exit 1
fi

# Install new dependencies
echo "📦 Installing new dependencies..."
npm install

# Check if installation was successful
if [ $? -eq 0 ]; then
    echo "✅ Dependencies installed successfully!"
else
    echo "❌ Failed to install dependencies"
    exit 1
fi

# Create .env file if it doesn't exist
if [ ! -f ".env" ]; then
    echo "🔧 Creating .env file..."
    cat > .env << EOF
# Audio API Configuration
REACT_APP_API_URL=http://192.168.1.203:8000

# Audio Features
REACT_APP_AUDIO_ENABLED=true
REACT_APP_AUDIO_UPLOAD_MAX_SIZE=104857600
EOF
    echo "✅ .env file created"
else
    echo "ℹ️  .env file already exists"
fi

echo ""
echo "🎉 Audio setup complete!"
echo ""
echo "🚀 To start the application:"
echo "   npm start"
echo ""
echo "📱 Audio features available at:"
echo "   • /audio - Audio Dashboard"
echo "   • /audio/library - Music Library"
echo "   • /audio/playlists - Playlist Management"
echo "   • /audio/upload - Music Upload"
echo "   • /audio/search - Advanced Search"
echo "   • /audio/settings - Audio Settings"
echo ""
echo "🔗 Make sure your Raspberry Pi audio server is running on http://192.168.1.203:8000"
