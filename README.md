# TiddlyPom
## _A simple console Pomodoro for C++ neckbeards_

### Why?
I wanted a simple, UNIX-friendly [Pomodoro](http://pomodorotechnique.com/) timer.

So I came across [Thyme](http://thymerb.com) and, while simple, it requires (a) a whole Ruby stack, (b) a bunch of dependencies and (c) does the annoying thing of writing to a file for integration elsewhere. A cute trick, but a dirty one nonetheless.

There's a right way to do this. UNIX domain sockets! C++!

So here it is. 

### Building
Clone the repository. In the directory, run

```
make
```

And assuming you have g++ and a unixy system, it will just work. Nothing special.

### Usage
If you try

```
./tpom
```

You'll see a blank line. This is because the timer isn't running. Try

```
./tpom start
```

To start the daemon for a default of 25 minutes. If you want a different amount of time try one of:

```
./tpom start -s 90
./tpom start -m 15
```

For 90 seconds or 15 minutes, respectively. Now that it's running, run

```
./tpom
```

And lo, the remaining time will be echoed back to you.

### Integration
Because just running tpom is sufficient to connect to the daemon and get the remaining time (without smashing your filesystem cache) -- running the command means you can integrate it wherever you need. 

#### tmux
Add

```
set -g status-right '#(~/path/to/tpom/tpom)'
set -g status-interval 1
```

Potentially with other options for your status-right, potentially other colors -- mine, for instance is:

```
set -g status-right '#[fg=green]#(~/src/tpom/tpom) #[fg=cyan,bold]%Y-%m-%d %H:%M:%S#[default]'
```

But you can fit it to taste.

#### awesome
I love [AwesomeWM](http://awesome.naquadah.org/). So you can add the timer as a countdown widget in your ~/.config/awesome/rc.lua:

```
pom = widget({type = "textbox"})
pom.text = " " .. execute_command("~/path/to/tpom") .. " "
pomtimer = timer({ timeout = 1})
pomtimer:add_signal("timeout", function() 
  pom.text = " " .. execute_command("~/path/to/tpom") .. " " 
end)
pomtimer:start()
```

And add the ```pom``` variable to your widgets list.

### Post-Hook

Not to be outdone, you can execute an arbitrary script when the timer goes off. This is totally useful to cause notifications to fly, ring an alarm -- whatever you like. Create an executable file at ```~/.tpm-post.sh``` with a standard ```#!``` line at the top. An example is included -- just copy it to the right place.

### Questions?
It's a tiny thing, but feel free to help me improve it. Drop me a line, or [follow me on Twitter](http://twitter.com/barakmich). 
