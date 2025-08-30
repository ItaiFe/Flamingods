#!/bin/bash

# Flamingods Audio React App Setup Script
# This script helps you set up the React audio control app

set -e

echo "🎵 Flamingods Audio React App Setup"
echo "=================================="
echo ""

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo "❌ Node.js is not installed. Please install Node.js 16+ first."
    echo "   Visit: https://nodejs.org/"
    exit 1
fi

# Check Node.js version
NODE_VERSION=$(node -v | cut -d'v' -f2 | cut -d'.' -f1)
if [ "$NODE_VERSION" -lt 16 ]; then
    echo "❌ Node.js version 16+ is required. Current version: $(node -v)"
    echo "   Please upgrade Node.js and try again."
    exit 1
fi

echo "✅ Node.js $(node -v) detected"
echo "✅ npm $(npm -v) detected"
echo ""

# Install dependencies
echo "📦 Installing dependencies..."
npm install

if [ $? -eq 0 ]; then
    echo "✅ Dependencies installed successfully"
else
    echo "❌ Failed to install dependencies"
    exit 1
fi

echo ""

# Create environment file
echo "🔧 Creating environment configuration..."
if [ ! -f .env ]; then
    cat > .env << EOF
# Flamingods Audio React App Configuration
# API endpoint for your Raspberry Pi audio system
REACT_APP_API_URL=http://localhost:8000

# Optional: Customize the app title
REACT_APP_TITLE=Flamingods Audio Control

# Optional: Enable debug logging
REACT_APP_DEBUG=false
EOF
    echo "✅ Created .env file with default configuration"
else
    echo "ℹ️  .env file already exists"
fi

echo ""

# Check if audio system is accessible
echo "🔍 Checking audio system connectivity..."
if command -v curl &> /dev/null; then
    if curl -s http://localhost:8000/audio/health > /dev/null 2>&1; then
        echo "✅ Audio system is accessible at http://localhost:8000"
    else
        echo "⚠️  Audio system is not accessible at http://localhost:8000"
        echo "   Make sure your Raspberry Pi audio system is running"
        echo "   You can update the API URL in .env file"
    fi
else
    echo "ℹ️  curl not available, skipping connectivity check"
fi

echo ""

# Create start script
echo "🚀 Creating start script..."
cat > start.sh << 'EOF'
#!/bin/bash

# Start the Flamingods Audio React App
echo "🎵 Starting Flamingods Audio React App..."
echo "🌐 App will be available at: http://localhost:3000"
echo "🔌 Make sure your audio system is running on the configured API endpoint"
echo ""

# Check if .env exists and load it
if [ -f .env ]; then
    echo "📋 Loading configuration from .env file..."
    export $(cat .env | grep -v '^#' | xargs)
fi

# Start the development server
npm start
EOF

chmod +x start.sh
echo "✅ Created start.sh script"

echo ""

# Create build script
echo "🏗️  Creating build script..."
cat > build.sh << 'EOF'
#!/bin/bash

# Build the Flamingods Audio React App for production
echo "🏗️  Building Flamingods Audio React App..."

# Check if .env exists and load it
if [ -f .env ]; then
    echo "📋 Loading configuration from .env file..."
    export $(cat .env | grep -v '^#' | xargs)
fi

# Build the app
npm run build

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build completed successfully!"
    echo "📁 Production files are in the 'build' directory"
    echo "🌐 You can serve these files with any web server"
    echo ""
    echo "💡 To serve with Python:"
    echo "   cd build && python -m http.server 3000"
    echo ""
    echo "💡 To serve with Node.js:"
    echo "   npm install -g serve && serve -s build -l 3000"
else
    echo "❌ Build failed"
    exit 1
fi
EOF

chmod +x build.sh
echo "✅ Created build.sh script"

echo ""

# Display next steps
echo "🎉 Setup completed successfully!"
echo ""
echo "📋 Next steps:"
echo "1. Update .env file with your audio system API URL if needed"
echo "2. Start the development server: ./start.sh"
echo "3. Open your browser to: http://localhost:3000"
echo "4. Build for production: ./build.sh"
echo ""
echo "📚 For more information, see README.md"
echo "🐛 For troubleshooting, check the console and network tab"
echo ""
echo "🎵 Happy listening with Flamingods Audio!"
