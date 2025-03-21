# Styx3D

Tryna rework Genesis3D to work on modern systems, port it to some homebrew consoles, like DreamCast and PSP, and maybe add some features that will improve QOL, like perhaps scripting.

## Rationale

There aren't really any other engines out there that are of this vintage under permissive licenses with the following feature set:

- Skeletal models/animations
- BSP 3D level format which can be modeled in a brush-based editor
- Fixed function pipeline 

## Plans

- [ ] Get it compiling on Linux & other modern systems (in progress)
  
  - Currently working on the engine in `./OpenSource/Source`

- [ ] Port to SDL

- [ ] Implement an OpenGL 1.1 rendering backend

- [ ] Get it ported to DreamCast and PSP

- [ ] Add support for game data being stored in a compressed format, like PK3 or WAD

- [ ] Create command-line versions of the tools

- [ ] Maybe add scripting support (Lua or Umka)

## License

Genesis3D was originally distributed under the Genesis3D public license, however, according to the [Genesis3D website](https://www.genesis3d.com/), the terms of this license have been waived:

> If you care to develop with Genesis3D 1.1, you may do so, free of licensing obligations as the engine is abandonware and the stakeholders are long gone.

As such, I am redistributing this as public domain software, dual licensed under the terms of the Zero-Clause BSD license, as some territories do not legally recognize the public domain.

The terms of the Zero-Clause BSD license are thus:

```
Copyright (C) 2025 Christopher DeBoy <chrisxdeboy@gmail.com>


Permission to use, copy, modify, and/or distribute this software for  
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL  
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES  
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE  
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY  
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN  
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT  
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```
