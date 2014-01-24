{
  'variables': {
    'ngn_shared_cares%': 'false',
    'ngn_shared_libuv%': 'false',
    'ngn_optional_polyfill%': 'true'
  },
  'targets': [
    {
      'target_name': 'ngn',
      'type': 'executable',
      'msvs_guid': '02410d86-8b77-471f-92b9-241d504a678f',
      'dependencies': [
        'deps/uv/uv.gyp:libuv',
      ],
      'defines': [
        'ARCH="<(target_arch)"',
        'PLATFORM="<(OS)"',
        ''
        'DEFINE_A_VALUE=value',
      ],
      'include_dirs': [
        'src',
        'deps/uv/src/ares'
      ],
     'sources': [
        'src/allocation.cpp',
        'src/api.cpp',
        'src/buffer.cpp',
        'src/checks.cpp',
        'src/encoding.cpp',
        'src/eventloop.cpp',
        'src/filesystem.cpp',
        'src/main.cpp',
        'src/platform-posix.cpp',
        'src/scanner-character-streams.cpp',
        'src/stream.cpp',
        'src/string_bytes.cpp',
        'src/unicode.cpp',
        'src/utils.cpp',
        'src/v8utils.cpp',
        'src/wrapper.cpp',
        # headers for IDE
        'src/allocation.h',
        'src/assert-scope.h',
        'src/atomicops.h',
        'src/atomicops_internals_x86_macosx.h',
        'src/buffer.h',
        'src/char-predicates.h',
        'src/checks.cpp',
        'src/checks.h',
        'src/encoding.h',
        'src/event.h',
        'src/eventloop.h',
        'src/exceptions.h',
        'src/filesystem.h',
        'src/globals.h',
        'src/hashmap.h',
        'src/internals.h',
        'src/list.h',
        'src/ngn.h',
        'src/platform.h',
        'src/pointer_iterator.h',
        'src/scanner-character-streams.cpp',
        'src/scanner-character-streams.h',
        'src/scanner.h',
        'src/stream.h',
        'src/string_bytes.h',
        'src/traits.h',
        'src/unicode-inl.h',
        'src/unicode.h',
        'src/utils.h',
        'src/v8.h',
        'src/v8checks.h',
        'src/v8config.h',
        'src/v8globals.h',
        'src/v8stdint.h',
        'src/v8utils.h',
        'src/wrapper.h',
      ],
      'conditions': [
        ['ngn_optional_polyfill=="true"', {
          'defines': ['NGN_OPTIONAL_POLYFILL'],
          'sources': ['src/optional.h']
        }],
        ['OS=="linux"', {
          'defines': [
          ],
          'include_dirs': [
          ],
        }],
        ['OS=="win"', {
          'defines': [
          ],
        }, { # OS != "win",
          'defines': [
          ],
        }]
      ],
    },
  ],
  }
