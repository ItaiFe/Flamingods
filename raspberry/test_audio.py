#!/usr/bin/env python3
"""
Test script for the audio system

This script tests the audio manager and API endpoints to ensure
the music playback system is working correctly.
"""

import asyncio
import sys
import os
import time

# Add the current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from audio_manager import AudioManager
from audio_models import AudioConfig, AudioControl
from config import get_config

async def test_audio_system():
    """Test the audio system functionality"""
    
    print("🎵 Testing Audio System")
    print("=" * 40)
    
    # Get configuration
    config = get_config()
    print(f"📋 Audio Configuration:")
    print(f"   - Music folder: {config.audio.music_folder}")
    print(f"   - Playlist folder: {config.audio.playlist_folder}")
    print(f"   - Supported formats: {config.audio.supported_formats}")
    print(f"   - Default volume: {config.audio.default_volume}%")
    print(f"   - Sample rate: {config.audio.sample_rate} Hz")
    print(f"   - Channels: {config.audio.channels}")
    print()
    
    # Create audio manager
    print("🔧 Creating Audio Manager...")
    audio_manager = AudioManager(config.audio)
    
    try:
        # Start audio manager
        print("🚀 Starting Audio Manager...")
        await audio_manager.start()
        
        # Test library scanning
        print("🔍 Testing library scan...")
        scan_result = await audio_manager.scan_music_library()
        print(f"   Scan result: {scan_result.success}")
        if scan_result.success:
            print(f"   Tracks found: {scan_result.data.get('tracks_count', 0)}")
            print(f"   Playlists found: {scan_result.data.get('playlists_count', 0)}")
            print(f"   Scan duration: {scan_result.data.get('scan_duration', 0):.2f} seconds")
        else:
            print(f"   Scan failed: {scan_result.error}")
        
        print()
        
        # Display library information
        tracks = audio_manager.get_tracks()
        playlists = audio_manager.get_playlists()
        
        print(f"📚 Library Information:")
        print(f"   - Total tracks: {len(tracks)}")
        print(f"   - Total playlists: {len(playlists)}")
        
        if tracks:
            print(f"\n📱 Sample Tracks:")
            for i, track in enumerate(tracks[:5], 1):  # Show first 5 tracks
                print(f"   {i}. {track.title}")
                print(f"      - Artist: {track.artist or 'Unknown'}")
                print(f"      - Album: {track.album or 'Unknown'}")
                print(f"      - Format: {track.format.value}")
                print(f"      - Duration: {track.duration_seconds or 0:.1f}s")
                print()
        
        if playlists:
            print(f"📋 Sample Playlists:")
            for i, playlist in enumerate(playlists[:3], 1):  # Show first 3 playlists
                print(f"   {i}. {playlist.name}")
                print(f"      - Tracks: {len(playlist.tracks)}")
                print(f"      - Duration: {playlist.total_duration or 0:.1f}s")
                print(f"      - Shuffle: {playlist.shuffle}")
                print(f"      - Repeat: {playlist.repeat}")
                print()
        
        # Test playback control (if tracks available)
        if tracks:
            print("🎛️  Testing Playback Control...")
            
            # Test volume control
            print("   🔊 Testing volume control...")
            volume_result = audio_manager.set_volume(80)
            print(f"      Volume set to 80%: {volume_result.success}")
            
            # Test mute toggle
            print("   🔇 Testing mute toggle...")
            mute_result = audio_manager.toggle_mute()
            print(f"      Mute toggled: {mute_result.success}")
            
            # Unmute
            audio_manager.toggle_mute()
            
            # Test track playback (first track)
            first_track = tracks[0]
            print(f"   ▶️  Testing track playback: {first_track.title}")
            
            play_result = await audio_manager.play_track(first_track.id)
            print(f"      Play result: {play_result.success}")
            
            if play_result.success:
                # Wait a moment for playback to start
                await asyncio.sleep(2)
                
                # Check status
                status = audio_manager.get_playback_status()
                print(f"      Playback state: {status.state.value}")
                print(f"      Current track: {status.current_track.title if status.current_track else 'None'}")
                print(f"      Position: {status.position_seconds:.1f}s / {status.duration_seconds:.1f}s")
                print(f"      Volume: {status.volume}%")
                print(f"      Muted: {status.muted}")
                
                # Test pause
                print("   ⏸️  Testing pause...")
                pause_result = await audio_manager.pause_playback()
                print(f"      Pause result: {pause_result.success}")
                
                await asyncio.sleep(1)
                
                # Test resume
                print("   ▶️  Testing resume...")
                resume_result = await audio_manager.resume_playback()
                print(f"      Resume result: {resume_result.success}")
                
                await asyncio.sleep(2)
                
                # Test stop
                print("   ⏹️  Testing stop...")
                stop_result = await audio_manager.stop_playback()
                print(f"      Stop result: {stop_result.success}")
                
                # Final status
                final_status = audio_manager.get_playback_status()
                print(f"      Final state: {final_status.state.value}")
        
        # Test audio statistics
        print("\n📊 Audio Statistics:")
        stats = audio_manager.get_audio_stats()
        print(f"   - Total tracks: {stats.total_tracks}")
        print(f"   - Total playlists: {stats.total_playlists}")
        print(f"   - Total duration: {stats.total_duration:.1f} hours")
        print(f"   - Library size: {stats.library_size_bytes / (1024*1024):.1f} MB")
        print(f"   - Tracks played: {stats.tracks_played}")
        print(f"   - Total play time: {stats.total_play_time:.1f} hours")
        print(f"   - System uptime: {stats.uptime_seconds:.1f} seconds")
        print(f"   - Errors encountered: {stats.errors_count}")
        
        # Test health check
        print("\n🏥 Audio System Health:")
        health_status = {
            "initialized": audio_manager.is_initialized,
            "playback_state": audio_manager.playback_state.value,
            "volume": audio_manager.volume,
            "muted": audio_manager.muted,
            "tracks_loaded": len(audio_manager.tracks),
            "playlists_loaded": len(audio_manager.playlists)
        }
        
        for key, value in health_status.items():
            print(f"   - {key}: {value}")
        
        print("\n✅ Audio system test completed successfully!")
        
    except Exception as e:
        print(f"❌ Error during audio system test: {e}")
        import traceback
        traceback.print_exc()
    
    finally:
        # Stop audio manager
        print("\n🛑 Stopping Audio Manager...")
        await audio_manager.stop()
        print("✅ Audio Manager stopped")


def main():
    """Main function"""
    print("🎵 Audio System Test")
    print("=" * 25)
    print()
    
    # Run the async test
    asyncio.run(test_audio_system())


if __name__ == "__main__":
    main()
