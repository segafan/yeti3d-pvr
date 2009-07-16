This example shows how to use Quake 2 (*.md2) models instead of sprites.

Notice that this has its own copy of game.c. Compare it with the original
game.c in the src directory to see important differences.  I may at some
point make a yeti_md2_entity function to do most of this stuff.

Also, this example will be useful to those who want to add something to the
game that would require adding it to the entity struct.  The "tag" in the
entity structure allows you to do this.  In this example, we use it to keep
information about the entity's model but you could use it for anything.
