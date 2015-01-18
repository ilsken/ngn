{
  'variables': {
    'ngn_shared_cares%': 'false',
    'ngn_shared_libuv%': 'false',
    'ngn_use_boost%': 'false',
    # set to false if your compiler supports std::optional
    'ngn_use_optional_standalone%': 'true',
    # set a custom location to search for boost headers
    'ngn_use_custom_boost_root%': 'false',
    'ngn_custom_boost_root%': 'deps/boost',
    'ngn_enable_il8n_support%': 'false',
    'icu_gyp_path%': 'deps/icu/icu.gyp',
    'os_posix%': 1
  },
  'targets': [
    {
      'target_name': 'ngn',
      'type': 'executable',
      'msvs_guid': '02410d86-8b77-471f-92b9-241d504a678f',
      'dependencies': [
        'deps/uv/uv.gyp:libuv',
        'deps/tq/tq.gyp:tq'
      ],
      'defines': [
        'ARCH="<(target_arch)"',
        'PLATFORM="<(OS)"'
      ],
      'include_dirs': [
        'src/',
        'deps/uv/src/ares',
        'deps/core/include'
      ],
     'sources': [
        'src/buffer.cpp',
        'src/buffer_decoder.cpp',
        'src/encoding.cpp',
        'src/eventloop.cpp',
        'src/filesystem.cpp',
      #  'src/folly/io/IOBuf.cpp',
        'src/handle.cpp',
        'src/io_buffer.cpp',
        'src/main.cpp',
        'src/stream.cpp',
        'src/string_bytes.cpp',
        'src/unicode_string.cpp',
        'src/utils.cpp',
        'src/wrapper.cpp',

        # headers for IDE
        'src/any-standalone.h',
        'src/any.h',
        'src/buffer.h',
        'src/buffer_decoder.h',
        'src/boost/config.hpp',
        'src/encoding.h',
        'src/event.h',
        'src/eventloop.h',
        'src/exceptions.h',
        'src/filesystem.h',
        'src/folly/config.h',
        'src/folly/detail/UncaughtExceptionCounter.h',
       # 'src/folly/io/IOBuf.h',
        'src/folly/Likely.h',
        'src/folly/Optional.h',
        'src/folly/Preprocessor.h',
        'src/folly/ScopeGuard.h',
        'src/handle.h',
        'src/io_buffer.h',
        'src/ngn.h',
        'src/optional-standalone.h',
        'src/optional.h',
        'src/pointer_iterator.h',
        'src/stream.h',
        'src/string_bytes.h',
        'src/traits.h',
        'src/unicode_string.h',
        'src/utils.h',
        'src/wrapper.h'
        ],
      'conditions': [
        ['ngn_enable_il8n_support == "true"', {
          'dependencies': [
            '<(icu_gyp_path):icui18n',
            '<(icu_gyp_path):icuuc',
            '<(icu_gyp_path):icudata'
          ]
        }
        ],
        ['ngn_use_boost=="true"', {
          'defines': ['NGN_USE_BOOST']
        }],
        ['ngn_use_boost=="true" and ngn_use_custom_boost_root=="true"', {
          'include_dirs': ['<(ngn_custom_boost_root)']
        }],
        ['ngn_use_optional_standalone=="true" and ngn_use_boost=="false"', {
          'defines': ['NGN_USE_OPTIONAL_STANDALONE'],
          'sources': ['src/optional-standalone.h']
        }],
        ['ngn_use_boost=="false"', {
          'sources': ['src/any-standalone.h']
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
