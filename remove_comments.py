#!/usr/bin/env python3
"""
Script to remove C++ comments from source files.
Handles both single-line (//) and multi-line (/* */) comments.
Preserves string literals and character literals.
"""

import os
import re
import sys
from pathlib import Path


def remove_cpp_comments(code):
    """
    Remove C++ comments from code while preserving strings and character literals.
    
    Args:
        code: The source code string
        
    Returns:
        Code with comments removed
    """
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " "  # Replace comment with space to preserve formatting
        else:
            return s  # Keep string/char literals
    
    # Pattern that matches:
    # - String literals: "..."
    # - Character literals: '...'
    # - Single-line comments: //...
    # - Multi-line comments: /* ... */
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    
    return re.sub(pattern, replacer, code)


def clean_empty_lines(code):
    """
    Remove excessive empty lines (more than 2 consecutive).
    
    Args:
        code: The source code string
        
    Returns:
        Code with cleaned up empty lines
    """
    # Replace 3+ consecutive newlines with 2 newlines
    return re.sub(r'\n{3,}', '\n\n', code)


def process_file(filepath, dry_run=False):
    """
    Process a single file to remove comments.
    
    Args:
        filepath: Path to the file
        dry_run: If True, only show what would be changed
        
    Returns:
        True if file was modified (or would be modified in dry_run)
    """
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            original_content = f.read()
        
        # Remove comments
        cleaned_content = remove_cpp_comments(original_content)
        
        # Clean up excessive empty lines
        cleaned_content = clean_empty_lines(cleaned_content)
        
        # Check if file changed
        if original_content != cleaned_content:
            if dry_run:
                print(f"[DRY RUN] Would modify: {filepath}")
                return True
            else:
                with open(filepath, 'w', encoding='utf-8', errors='ignore') as f:
                    f.write(cleaned_content)
                print(f"✓ Processed: {filepath}")
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
