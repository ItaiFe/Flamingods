#!/usr/bin/env python3
"""
Test script for audio file upload functionality

This script tests the upload endpoints to ensure they work correctly
with various file types and scenarios.
"""

import asyncio
import sys
import os
import tempfile
from pathlib import Path

# Add the current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import httpx
from config import get_config

async def test_upload_endpoints():
    """Test the upload endpoints"""
    
    print("📤 Testing Audio File Upload Endpoints")
    print("=" * 50)
    
    # Get configuration
    config = get_config()
    base_url = f"http://{config.server.host}:{config.server.port}"
    
    print(f"🌐 Server URL: {base_url}")
    print(f"📁 Music folder: {config.audio.music_folder}")
    print(f"🎵 Supported formats: {config.audio.supported_formats}")
    print()
    
    # Test server health first
    print("🏥 Testing server health...")
    try:
        async with httpx.AsyncClient() as client:
            response = await client.get(f"{base_url}/audio/health")
            if response.status_code == 200:
                print("   ✅ Server is healthy")
                health_data = response.json()
                print(f"   📊 Audio system initialized: {health_data.get('audio_system', {}).get('initialized', False)}")
            else:
                print(f"   ❌ Server health check failed: {response.status_code}")
                return
    except Exception as e:
        print(f"   ❌ Cannot connect to server: {e}")
        print("   💡 Make sure the server is running: uv run python main.py")
        return
    
    print()
    
    # Test single file upload
    print("📁 Testing single file upload...")
    await test_single_upload(base_url)
    
    print()
    
    # Test batch file upload
    print("📦 Testing batch file upload...")
    await test_batch_upload(base_url)
    
    print()
    
    # Test file deletion
    print("🗑️  Testing file deletion...")
    await test_file_deletion(base_url)
    
    print()
    
    # Test scan uploaded files
    print("🔍 Testing scan uploaded files...")
    await test_scan_uploaded(base_url)
    
    print("\n✅ Upload testing completed!")


async def test_single_upload(base_url: str):
    """Test single file upload"""
    try:
        # Create a test audio file (simulated)
        test_file_content = b"fake_audio_data_for_testing"
        
        # Test with different file types
        test_files = [
            ("test_song.mp3", "mp3", "electronic"),
            ("ambient_track.wav", "wav", "ambient"),
            ("beat.flac", "flac", "electronic"),
            ("chill.ogg", "ogg", "ambient")
        ]
        
        async with httpx.AsyncClient() as client:
            for filename, format_type, category in test_files:
                print(f"   📤 Uploading {filename} to {category} category...")
                
                # Create form data
                files = {"file": (filename, test_file_content, f"audio/{format_type}")}
                data = {"category": category}
                
                try:
                    response = await client.post(
                        f"{base_url}/audio/upload",
                        files=files,
                        data=data
                    )
                    
                    if response.status_code == 200:
                        result = response.json()
                        print(f"      ✅ Upload successful: {result.get('message', 'Unknown')}")
                        if result.get('data'):
                            data_info = result['data']
                            print(f"         - Track ID: {data_info.get('track_id', 'Unknown')}")
                            print(f"         - Title: {data_info.get('title', 'Unknown')}")
                            print(f"         - Category: {data_info.get('category', 'Unknown')}")
                    else:
                        print(f"      ❌ Upload failed: {response.status_code}")
                        print(f"         - Response: {response.text}")
                        
                except Exception as e:
                    print(f"      ❌ Upload error: {e}")
                
                # Small delay between uploads
                await asyncio.sleep(0.5)
    
    except Exception as e:
        print(f"   ❌ Single upload test failed: {e}")


async def test_batch_upload(base_url: str):
    """Test batch file upload"""
    try:
        # Create multiple test files
        test_files = [
            ("batch_song1.mp3", b"fake_audio_data_1", "electronic"),
            ("batch_song2.wav", b"fake_audio_data_2", "electronic"),
            ("batch_ambient1.flac", b"fake_audio_data_3", "ambient")
        ]
        
        print("   📦 Preparing batch upload...")
        
        async with httpx.AsyncClient() as client:
            # Create form data for batch upload
            files = []
            for filename, content, category in test_files:
                files.append(("files", (filename, content, "audio/mpeg")))
            
            data = {"category": "batch_test"}
            
            try:
                response = await client.post(
                    f"{base_url}/audio/upload/batch",
                    files=files,
                    data=data
                )
                
                if response.status_code == 200:
                    result = response.json()
                    print(f"      ✅ Batch upload successful: {result.get('message', 'Unknown')}")
                    if result.get('data'):
                        data_info = result['data']
                        print(f"         - Uploaded: {data_info.get('uploaded_count', 0)} files")
                        print(f"         - Failed: {data_info.get('failed_count', 0)} files")
                        print(f"         - Category: {data_info.get('category', 'Unknown')}")
                        
                        # Show uploaded files
                        uploaded_files = data_info.get('uploaded_files', [])
                        if uploaded_files:
                            print("         📁 Uploaded files:")
                            for file_info in uploaded_files:
                                print(f"            - {file_info.get('title', 'Unknown')} ({file_info.get('filename', 'Unknown')})")
                        
                        # Show failed files
                        failed_files = data_info.get('failed_files', [])
                        if failed_files:
                            print("         ❌ Failed files:")
                            for file_info in failed_files:
                                print(f"            - {file_info.get('filename', 'Unknown')}: {file_info.get('error', 'Unknown error')}")
                else:
                    print(f"      ❌ Batch upload failed: {response.status_code}")
                    print(f"         - Response: {response.text}")
                    
            except Exception as e:
                print(f"      ❌ Batch upload error: {e}")
    
    except Exception as e:
        print(f"   ❌ Batch upload test failed: {e}")


async def test_file_deletion(base_url: str):
    """Test file deletion"""
    try:
        print("   🗑️  Testing file deletion...")
        
        async with httpx.AsyncClient() as client:
            # First, get the list of tracks
            response = await client.get(f"{base_url}/audio/tracks")
            
            if response.status_code == 200:
                tracks = response.json()
                if tracks:
                    # Try to delete the first track
                    first_track = tracks[0]
                    track_id = first_track.get('id')
                    track_title = first_track.get('title', 'Unknown')
                    
                    print(f"      🎯 Attempting to delete: {track_title}")
                    
                    delete_response = await client.delete(f"{base_url}/audio/tracks/{track_id}")
                    
                    if delete_response.status_code == 200:
                        result = delete_response.json()
                        print(f"         ✅ Deletion successful: {result.get('message', 'Unknown')}")
                    else:
                        print(f"         ❌ Deletion failed: {delete_response.status_code}")
                        print(f"            - Response: {delete_response.text}")
                else:
                    print("      ℹ️  No tracks available for deletion test")
            else:
                print(f"      ❌ Failed to get tracks: {response.status_code}")
    
    except Exception as e:
        print(f"   ❌ File deletion test failed: {e}")


async def test_scan_uploaded(base_url: str):
    """Test scan uploaded files endpoint"""
    try:
        print("   🔍 Testing scan uploaded files...")
        
        async with httpx.AsyncClient() as client:
            response = await client.post(f"{base_url}/audio/scan/uploaded")
            
            if response.status_code == 200:
                result = response.json()
                print(f"      ✅ Scan successful: {result.get('message', 'Unknown')}")
                if result.get('data'):
                    data_info = result['data']
                    print(f"         - Tracks found: {data_info.get('tracks_count', 0)}")
                    print(f"         - Playlists found: {data_info.get('playlists_count', 0)}")
                    print(f"         - Scan duration: {data_info.get('scan_duration', 0):.2f} seconds")
            else:
                print(f"      ❌ Scan failed: {response.status_code}")
                print(f"         - Response: {response.text}")
    
    except Exception as e:
        print(f"   ❌ Scan uploaded files test failed: {e}")


async def test_upload_with_real_files(base_url: str):
    """Test upload with actual audio files (if available)"""
    print("\n🎵 Testing upload with real audio files...")
    
    # Check if there are any audio files in the current directory
    audio_extensions = ['.mp3', '.wav', '.flac', '.ogg']
    current_dir = Path('.')
    
    audio_files = []
    for ext in audio_extensions:
        audio_files.extend(current_dir.glob(f"*{ext}"))
        audio_files.extend(current_dir.glob(f"*{ext.upper()}"))
    
    if not audio_files:
        print("   ℹ️  No audio files found in current directory for testing")
        print("   💡 Place some .mp3, .wav, .flac, or .ogg files here to test real uploads")
        return
    
    print(f"   📁 Found {len(audio_files)} audio files for testing")
    
    async with httpx.AsyncClient() as client:
        for audio_file in audio_files[:3]:  # Test first 3 files
            print(f"   📤 Uploading real file: {audio_file.name}")
            
            try:
                with open(audio_file, 'rb') as f:
                    files = {"file": (audio_file.name, f.read(), "audio/mpeg")}
                    data = {"category": "real_files_test"}
                    
                    response = await client.post(
                        f"{base_url}/audio/upload",
                        files=files,
                        data=data
                    )
                    
                    if response.status_code == 200:
                        result = response.json()
                        print(f"      ✅ Upload successful: {result.get('message', 'Unknown')}")
                    else:
                        print(f"      ❌ Upload failed: {response.status_code}")
                        
            except Exception as e:
                print(f"      ❌ Upload error: {e}")
            
            await asyncio.sleep(1)  # Delay between uploads


def main():
    """Main function"""
    print("📤 Audio File Upload Test")
    print("=" * 30)
    print()
    
    # Run the async test
    asyncio.run(test_upload_endpoints())
    
    print("\n" + "=" * 50)
    print("💡 Upload Testing Tips:")
    print("   - Make sure the server is running: uv run python main.py")
    print("   - Check server logs for detailed error information")
    print("   - Verify the music folder has write permissions")
    print("   - Test with different file formats and categories")
    print("   - Use the /audio/scan endpoint after uploads to refresh the library")


if __name__ == "__main__":
    main()
