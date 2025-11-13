#!/usr/bin/env python3
"""
Script to normalize empty lines in C++ source files.
Converts multiple consecutive empty lines to single empty lines.
"""

import os
import re
import sys
from pathlib import Path


def normalize_empty_lines(code):
    """
    Replace multiple consecutive empty lines with single empty lines.
    Also removes trailing whitespace from lines.
    
    Args:
        code: The source code string
        
    Returns:
        Code with normalized empty lines
    """
    # First, remove trailing whitespace from each line
    lines = code.split('\n')
    lines = [line.rstrip() for line in lines]
    code = '\n'.join(lines)
    
    # Replace 2+ consecutive empty lines (3+ newlines) with single empty line (2 newlines)
    return re.sub(r'\n\n\n+', '\n\n', code)


def process_file(filepath, dry_run=False):
    """
    Process a single file to normalize empty lines.
    
    Args:
        filepath: Path to the file
        dry_run: If True, only show what would be changed
        
    Returns:
        True if file was modified (or would be modified in dry_run)
    """
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            original_content = f.read()
        
        # Normalize empty lines
        cleaned_content = normalize_empty_lines(original_content)
        
        # Check if file changed
        if original_content != cleaned_content:
            if dry_run:
                lines_removed = original_content.count('\n') - cleaned_content.count('\n')
                print(f"[DRY RUN] Would modify: {filepath} ({lines_removed} lines removed)")
                return True
            else:
                with open(filepath, 'w', encoding='utf-8', errors='ignore') as f:
                    f.write(cleaned_content)
                lines_removed = original_content.count('\n') - cleaned_content.count('\n')
                print(f"✓ Processed: {filepath} ({lines_removed} lines removed)")
                return True
        else:
            print(f"  No changes: {filepath}")
            return False
            
    except Exception as e:
        print(f"✗ Error processing {filepath}: {e}")
        return False


def main():
    """Main function to process all C++ files in the current directory."""
    
    # Parse arguments
    dry_run = '--dry-run' in sys.argv or '-n' in sys.argv
    
    if dry_run:
        print("=== DRY RUN MODE - No files will be modified ===\n")
    
    # Get current directory
    current_dir = Path('.')
    
    # File extensions to process
    extensions = ['.cpp', '.h', '.hpp', '.cc', '.cxx']
    
    # Find all C++ files (excluding SFML directory)
    cpp_files = []
    for ext in extensions:
        for file in current_dir.glob(f'*{ext}'):
            if 'SFML' not in str(file):
                cpp_files.append(file)
    
    if not cpp_files:
        print("No C++ files found in current directory.")
        return 0
    
    print(f"Found {len(cpp_files)} C++ file(s) to process\n")
    
    # Process each file
    modified_count = 0
    total_lines_removed = 0
    for filepath in sorted(cpp_files):
        if process_file(filepath, dry_run):
            modified_count += 1
    
    # Summary
    print(f"\n{'=' * 60}")
    if dry_run:
        print(f"Would modify {modified_count} file(s)")
        print("\nRun without --dry-run to apply changes")
    else:
        print(f"Successfully processed {modified_count} file(s)")
        print(f"No changes needed for {len(cpp_files) - modified_count} file(s)")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
