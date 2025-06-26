#!/usr/bin/env python3
#
# Build system detection module for Y2038 analysis
#
# This module provides comprehensive build system detection and parsing capabilities
# for extracting Y2038-related compiler flags from various build configuration files.
# It supports multiple build systems and implements a hierarchical search strategy
# to find the most relevant build configurations.
#
# Supported Build Systems:
# - GNU Make (Makefile, makefile, GNUmakefile, *.mk)
# - CMake (CMakeLists.txt, *.cmake)
# - Meson (meson.build)
# - Autotools (configure, configure.ac, configure.in)
# - Bazel (BUILD, BUILD.bazel)
# - Other build systems (Cargo.toml, package.json, setup.py)
#
# Key Features:
# - Configurable hierarchical directory search from source file
# - Multiple build system support with specialized parsers
# - Y2038-specific flag extraction (_TIME_BITS, _USE_TIME_BITS64, _FILE_OFFSET_BITS)
# - Robust error handling for malformed build files
# - Optional verbose mode for debugging
#
# Usage:
#   detector = BuildSystemDetector('/path/to/source.c', search_depth=3, verbose=True)
#   y2038_flags = detector.get_y2038_flags()
#   print(y2038_flags)  # {'_TIME_BITS': '64', '_FILE_OFFSET_BITS': '64'}
#

import re
import argparse
import codecs
import time
from pathlib import Path


class BuildSystemDetector:
    """
    Detects and parses build system files to extract compiler flags for Y2038 analysis.

    This class implements a comprehensive build system detection strategy that searches
    for build configuration files in a hierarchical manner, starting from the source
    file directory and moving up through parent directories. It supports multiple
    build systems and provides specialized parsers for each.

    The detector prioritizes build files found closer to the source file and merges
    flags from multiple build files when found, with later files taking precedence.

    Features efficient caching of parsed build files to avoid re-parsing unchanged
    files, with automatic cache invalidation based on file modification times.

    Attributes:
        source_file (Path): Path object representing the source file
        source_dir (Path): Directory containing the source file
        search_depth (int): How many parent directories to search for build files
        verbose (bool): Enable verbose output for debugging
        detected_flags (dict): Cache of detected compiler flags
        _file_cache (dict): Cache of parsed file contents with timestamps
        _cache_ttl (int): Cache time-to-live in seconds (default: 300)

    Example:
        >>> detector = BuildSystemDetector('/project/src/main.c', search_depth=3, verbose=True)
        >>> flags = detector.get_y2038_flags()
        {'_TIME_BITS': '64', '_FILE_OFFSET_BITS': '64'}
    """

    # Class-level cache shared between instances for better performance
    _global_file_cache = {}

    def __init__(self, source_file_path, search_depth=5, verbose=False, cache_ttl=300):
        """
        Initialize detector with a source file path.

        Sets up the detector to search for build files starting from the source file's
        directory and moving up through parent directories.

        Args:
            source_file_path (str): Path to the source file being analyzed
            search_depth (int, optional): How many parent directories to search. Defaults to 5.
            verbose (bool, optional): Enable verbose output for debugging. Defaults to False.
            cache_ttl (int, optional): Cache time-to-live in seconds. Defaults to 300 (5 minutes).

        Example:
            >>> detector = BuildSystemDetector('/home/user/project/src/main.c', search_depth=3)
            >>> detector.source_dir
            PosixPath('/home/user/project/src')
        """
        self.source_file = Path(source_file_path)
        self.source_dir = self.source_file.parent
        self.search_depth = search_depth
        self.verbose = verbose
        self.detected_flags = {}
        self._cache_ttl = cache_ttl

    def _is_cache_valid(self, file_path, cache_entry):
        """
        Check if a cache entry is still valid based on file modification time and TTL.

        Args:
            file_path (Path): Path to the file being checked
            cache_entry (dict): Cache entry containing 'mtime', 'content', and 'timestamp'

        Returns:
            bool: True if cache is valid, False otherwise
        """
        try:
            current_mtime = file_path.stat().st_mtime
            current_time = time.time()

            # Check if file has been modified or cache has expired
            return (cache_entry['mtime'] == current_mtime and
                    current_time - cache_entry['timestamp'] < self._cache_ttl)
        except OSError:
            # File doesn't exist or can't be accessed
            return False

    def _get_cached_content(self, file_path):
        """
        Get cached file content if available and valid.

        Args:
            file_path (Path): Path to the file

        Returns:
            str or None: Cached content if valid, None if cache miss or invalid
        """
        file_key = str(file_path.absolute())
        cache_entry = self._global_file_cache.get(file_key)

        if cache_entry and self._is_cache_valid(file_path, cache_entry):
            if self.verbose:
                print(f"Using cached content for {file_path}")
            return cache_entry['content']

        return None

    def _cache_content(self, file_path, content):
        """
        Cache file content with metadata.

        Args:
            file_path (Path): Path to the file
            content (str): File content to cache
        """
        try:
            file_key = str(file_path.absolute())
            self._global_file_cache[file_key] = {
                'content': content,
                'mtime': file_path.stat().st_mtime,
                'timestamp': time.time()
            }
        except OSError:
            # Can't get file stats, skip caching
            pass

    def _read_file_with_bom_handling(self, file_path):
        """
        Read file with proper UTF-8 BOM handling and caching for cross-platform compatibility.

        This method first checks for cached content, then attempts to read files with UTF-8
        encoding while properly handling the Byte Order Mark (BOM) that may be present on
        Windows systems. It tries multiple encoding strategies to ensure maximum compatibility
        and caches successful reads for better performance.

        Args:
            file_path (Path): Path to the file to read

        Returns:
            str: File content as string, or empty string if reading fails

        Raises:
            None: All exceptions are caught and handled gracefully
        """
        # Try to get content from cache first
        cached_content = self._get_cached_content(file_path)
        if cached_content is not None:
            return cached_content

        try:
            # First try UTF-8 with BOM detection
            with open(file_path, 'rb') as f:
                raw_data = f.read()

            # Check for UTF-8 BOM and remove it if present
            if raw_data.startswith(codecs.BOM_UTF8):
                raw_data = raw_data[len(codecs.BOM_UTF8):]

            # Decode as UTF-8
            content = raw_data.decode('utf-8', errors='ignore')

            # Cache the successful read
            self._cache_content(file_path, content)
            return content

        except (IOError, UnicodeDecodeError, OSError) as e:
            if self.verbose:
                print(f"Warning: Could not read {file_path} with BOM handling: {e}")

            # Fallback to simple UTF-8 reading with error ignoring
            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                # Cache the successful fallback read
                self._cache_content(file_path, content)
                return content

            except (IOError, OSError) as e2:
                if self.verbose:
                    print(f"Warning: Fallback read also failed for {file_path}: {e2}")
                return ""

    def detect_build_files(self):
        """
        Search for build system files starting from source directory and going up.

        Performs a hierarchical search for build configuration files, starting from
        the source file's directory and moving up through parent directories up to
        the configured search_depth. Supports various build system file patterns.

        Returns:
            list[Path]: List of Path objects representing found build files, ordered
                       by discovery (closest to source file first)

        Build file patterns searched:
            - Makefile variants: Makefile, makefile, GNUmakefile, *.mk
            - CMake files: CMakeLists.txt, *.cmake
            - Autotools: configure, configure.ac, configure.in
            - Meson: meson.build
            - Bazel: BUILD, BUILD.bazel
            - Others: Cargo.toml, package.json, setup.py
        """
        build_files = []
        current_dir = self.source_dir

        # Search up to search_depth levels up for build files
        for _ in range(self.search_depth):
            patterns = [
                'Makefile', 'makefile', 'GNUmakefile',
                'CMakeLists.txt',
                '*.mk', '*.cmake',
                'configure', 'configure.ac', 'configure.in',
                'meson.build', 'BUILD', 'BUILD.bazel',
                'Cargo.toml', 'package.json', 'setup.py'
            ]

            for pattern in patterns:
                # Use generator instead of list() for memory efficiency
                for match in current_dir.glob(pattern):
                    if match.is_file():
                        build_files.append(match)

            # Move to parent directory
            if current_dir.parent == current_dir:  # reached root
                break
            current_dir = current_dir.parent

        return build_files

    def parse_makefile(self, makefile_path):
        """
        Parse Makefile for compiler flags, specifically extracting -D definitions.

        Analyzes Makefile content to find compiler flag definitions in various
        variable assignments like CPPFLAGS, CXXFLAGS, CFLAGS, and override statements.
        Extracts -D preprocessor definitions and their values.

        Args:
            makefile_path (Path): Path to the Makefile to parse

        Returns:
            dict: Dictionary mapping preprocessor definition names to their values
                 (empty string if no value specified)
        """
        flags = {}
        try:
            content = self._read_file_with_bom_handling(makefile_path)
            if not content:
                return flags

            # Look for CPPFLAGS, CXXFLAGS, CFLAGS definitions, ignoring comments
            flag_patterns = [
                r'^\s*(?:override\s+)?(CPPFLAGS|CXXFLAGS|CFLAGS)\s*[:+]?=\s*(.+)'
            ]

            for pattern in flag_patterns:
                matches = re.findall(pattern, content, re.MULTILINE)
                for _, val in matches:
                    # Remove comments from the value
                    val = re.sub(r'#.*$', '', val)
                    # Extract -D definitions
                    define_matches = re.findall(r'-D([A-Za-z_][A-Za-z0-9_]*(?:=\S*)?)', val)
                    for define in define_matches:
                        if '=' in define:
                            key, value = define.split('=', 1)
                            flags[key] = value
                        else:
                            flags[define] = ''

        except Exception as e:
            if self.verbose:
                print(f"Warning: Could not parse {makefile_path}: {e}")

        return flags

    def parse_cmake(self, cmake_path):
        """
        Parse CMakeLists.txt for compiler flags, extracting -D definitions.

        Analyzes CMake configuration files to find compiler flag definitions in
        various CMake commands like add_definitions(), target_compile_definitions(),
        and set() commands for CMAKE_CXX_FLAGS or CMAKE_C_FLAGS.

        Args:
            cmake_path (Path): Path to the CMakeLists.txt or .cmake file to parse

        Returns:
            dict: Dictionary mapping preprocessor definition names to their values
                 (empty string if no value specified)
        """
        flags = {}
        try:
            content = self._read_file_with_bom_handling(cmake_path)
            if not content:
                return flags

            # Look for add_definitions, target_compile_definitions, set with compile flags
            patterns = [
                r'add_definitions\s*\(\s*([^)]+)\s*\)',
                r'target_compile_definitions\s*\([^)]*\s+(?:PRIVATE|PUBLIC|INTERFACE)\s+([^)]+)\)',
                r'set\s*\(\s*CMAKE_(?:CXX|C)_FLAGS[^)]*"([^"]*)"'
            ]

            for pattern in patterns:
                matches = re.findall(pattern, content, re.MULTILINE | re.DOTALL)
                for match in matches:
                    # Extract -D definitions or direct definitions
                    define_matches = re.findall(r'(?:-D)?([A-Za-z_][A-Za-z0-9_]*(?:=\S*)?)', match)
                    for define in define_matches:
                        if define.startswith('-'):
                            continue  # Skip flags that are not definitions
                        if '=' in define:
                            key, value = define.split('=', 1)
                            flags[key] = value
                        else:
                            flags[define] = ''

        except Exception as e:
            if self.verbose:
                print(f"Warning: Could not parse {cmake_path}: {e}")

        return flags

    def parse_configure_script(self, configure_path):
        """
        Parse configure script for compiler flags
        """
        flags = {}
        try:
            content = self._read_file_with_bom_handling(configure_path)
            if not content:
                return flags

            # Look for CPPFLAGS assignments and AC_DEFINE statements
            patterns = [
                r'CPPFLAGS="([^"]*)"',
                r"CPPFLAGS='([^']*)'",
                r'AC_DEFINE\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,\s*([^,)]+)'
            ]

            for pattern in patterns:
                matches = re.findall(pattern, content, re.MULTILINE)
                for match in matches:
                    if isinstance(match, tuple) and len(match) == 2:
                        # AC_DEFINE format
                        flags[match[0]] = match[1].strip('"\'')
                    else:
                        # CPPFLAGS format
                        define_matches = re.findall(r'-D([A-Za-z_][A-Za-z0-9_]*(?:=\S*)?)', match)
                        for define in define_matches:
                            if '=' in define:
                                key, value = define.split('=', 1)
                                flags[key] = value
                            else:
                                flags[define] = ''

        except Exception as e:
            if self.verbose:
                print(f"Warning: Could not parse {configure_path}: {e}")

        return flags

    def parse_meson_build(self, meson_path):
        """
        Parse meson.build for compiler flags
        """
        flags = {}
        try:
            content = self._read_file_with_bom_handling(meson_path)
            if not content:
                return flags

            # Look for add_global_arguments, add_project_arguments
            patterns = [
                r'add_global_arguments\s*\(\s*[\'"]([^\'\"]*)[\'"]',
                r'add_project_arguments\s*\(\s*[\'"]([^\'\"]*)[\'"]'
            ]

            for pattern in patterns:
                matches = re.findall(pattern, content, re.MULTILINE)
                for match in matches:
                    define_matches = re.findall(r'-D([A-Za-z_][A-Za-z0-9_]*(?:=\S*)?)', match)
                    for define in define_matches:
                        if '=' in define:
                            key, value = define.split('=', 1)
                            flags[key] = value
                        else:
                            flags[define] = ''

        except Exception as e:
            if self.verbose:
                print(f"Warning: Could not parse {meson_path}: {e}")

        return flags

    def extract_compiler_flags(self):
        """
        Main method to extract compiler flags from all detected build files.

        Orchestrates the entire build system detection and parsing process:
        1. Discovers build files using detect_build_files()
        2. Determines the appropriate parser for each file type
        3. Extracts compiler flags from each file
        4. Merges flags from multiple files (later files take precedence)
        5. Caches results in self.detected_flags

        Returns:
            dict: Complete dictionary of all detected compiler flags from all build files

        Note:
            Results are cached in self.detected_flags for subsequent calls.
        """
        build_files = self.detect_build_files()
        all_flags = {}

        for build_file in build_files:
            file_flags = {}
            filename = build_file.name.lower()

            if 'makefile' in filename or filename.endswith('.mk'):
                file_flags = self.parse_makefile(build_file)
            elif filename == 'cmakelists.txt' or filename.endswith('.cmake'):
                file_flags = self.parse_cmake(build_file)
            elif filename in ['configure', 'configure.ac', 'configure.in']:
                file_flags = self.parse_configure_script(build_file)
            elif filename == 'meson.build':
                file_flags = self.parse_meson_build(build_file)

            # Merge flags, with later files taking precedence
            all_flags.update(file_flags)

        self.detected_flags = all_flags
        return all_flags

    def get_y2038_flags(self):
        """
        Extract Y2038-specific flags from detected compiler flags.

        Filters the detected compiler flags to return only those relevant to Y2038
        analysis. This includes _TIME_BITS, _USE_TIME_BITS64, __USE_TIME_BITS64,
        and _FILE_OFFSET_BITS definitions.

        If flags haven't been detected yet, automatically calls extract_compiler_flags()
        to perform the detection first.

        Returns:
            dict: Dictionary containing only Y2038-related flags and their values

        Y2038-related flags:
            - _TIME_BITS: Controls time_t size (should be 64)
            - _USE_TIME_BITS64: Enables 64-bit time support
            - __USE_TIME_BITS64: Glibc internal flag for 64-bit time
            - _FILE_OFFSET_BITS: Controls file offset size (should be 64 for Y2038 safety)

        Example:
            >>> detector = BuildSystemDetector('/project/src/main.c')
            >>> y2038_flags = detector.get_y2038_flags()
            {'_TIME_BITS': '64', '_FILE_OFFSET_BITS': '64'}
        """
        if not self.detected_flags:
            self.extract_compiler_flags()

        y2038_flags = {}

        # Look for Y2038-related flags
        for flag, value in self.detected_flags.items():
            if flag in ['_TIME_BITS', '_USE_TIME_BITS64', '__USE_TIME_BITS64', '_FILE_OFFSET_BITS']:
                y2038_flags[flag] = value

        return y2038_flags

    def has_y2038_configuration(self):
        """
        Check if build system has Y2038-related configuration.

        Convenience method to quickly determine if any Y2038-related compiler flags
        are present in the detected build system files.

        Returns:
            bool: True if any Y2038-related flags are found, False otherwise
        """
        y2038_flags = self.get_y2038_flags()
        return len(y2038_flags) > 0

    @classmethod
    def clear_cache(cls):
        """
        Clear the global file cache.

        This method can be called to clear the shared cache, which might be useful
        for testing or when you want to ensure fresh reads of all build files.
        """
        cls._global_file_cache.clear()

    @classmethod
    def get_cache_stats(cls):
        """
        Get statistics about the current cache state.

        Returns:
            dict: Dictionary containing cache statistics like size and entries
        """
        return {
            'cache_size': len(cls._global_file_cache),
            'cached_files': list(cls._global_file_cache.keys())
        }


def detect_build_system_flags(source_file_path, search_depth=5, verbose=False):
    """
    Convenience function to detect Y2038-related flags from build systems.

    This is a simplified interface to the BuildSystemDetector class that creates
    a detector instance and immediately returns Y2038-specific flags.

    Args:
        source_file_path (str): Path to the source file being analyzed
        search_depth (int, optional): How many parent directories to search. Defaults to 5.
        verbose (bool, optional): Enable verbose output for debugging. Defaults to False.

    Returns:
        dict: Dictionary containing Y2038-related flags found in build system files
    """
    detector = BuildSystemDetector(source_file_path, search_depth=search_depth, verbose=verbose)
    return detector.get_y2038_flags()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Detect Y2038-related flags from build systems.')
    parser.add_argument('source_file', help='Path to the source file to analyze.')
    parser.add_argument('--depth', type=int, default=5, help='How many parent directories to search for build files.')
    parser.add_argument('--verbose', action='store_true', help='Enable verbose output for debugging.')
    args = parser.parse_args()

    flags = detect_build_system_flags(args.source_file, search_depth=args.depth, verbose=args.verbose)
    print("Detected Y2038-related flags:", flags)
