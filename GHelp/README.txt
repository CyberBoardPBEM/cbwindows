============
Ground Rules
============

If you're going to change existing areas of the help system, please let 
me know so we aren't both working on the same thing. Nobody likes to
work on stuff that is already being done by someone else.

Each person that wishes to contribute larger pieces of content would get
a snapshot of the help project "source code". They may do their edits, 
additions, or whatever and send me back the modified snapshot. This makes 
it easy for me to use the merge facilities in CVS to add the changes.

Also, it's important that the writer actually compile and test the file
against CB or in the standalone HTML help viewer. This is easy to do. You
just need the free HTML Help workshop. That is the only tool I'm using. It
also has an ok image capture program for getting snapshots of dialogs, etc.
You can download it from here:

http://msdn.microsoft.com/library/default.asp?URL=/library/tools/htmlhelp/chm/hh1start.htm

There is a general need to fill out the entire html help content. However,
I especially need help in these areas:

- The context help pop-ups for dialogs. These are stored in the text files
  named "gmcontext.txt" for the designer program and "gpcontext.txt" for
  the player program.

- Tutorials! One for the designer, one for the player. I've dedicated
  the first TOC entry for each program for these. The software truly
  needs these for new users.

- "How to..."'s. The how-to topics live under the "Advanced" heading 
  for each program. Share your expertise! If it's larger than a few
  paragraphs we can add a separate page just for that tip. Smaller tips
  can go in the general "How to" page.

Content ownership. If you contribute you must be willing to allow me to use
the content forever in the help file or possibly on the web site. This
would include edited versions of the same. You of course own what you do
too. I don't want to own what you do, I just want to be able to use it.

Significant content contributions will be noted on the cover page of the
help system. Smaller contributions such as a "How to" sheet will be noted
on the page itself. Really small tips in the general "How to" page will be
noted with those entries.

===========
Conventions
===========

Here's are various conventions to follow for Cyberboard's help system.

File Naming
-----------

The file names use a hierarchical format similar to the way usenet
newsgroups are presented: general moving toward specific. Instead of
periods, dashes are used to separate pieces. Use all lowercase characters
for file names.

The outermost level:

"gen-" : Overall information about CyberBoard
"gm-"  : Game designer program information
"gp-"  : Game player program information

Some examples of subdivisions that currently exist in the help system:

"gen-main-"
"gm-main-"
"gp-main-"  : Main body of info for the respective program area

"gm-ref-"
"gp-ref-"   : Detailed program references


"gm-tut-"
"gp-tut-"   : Program tutorials

"gm-adv-"
"gp-adv-"   : Advanced technique information

"gen-tech-" : technical info for the system

Bitmap images must be stored in the "images\" subdirectory. It should
follow essentially the same naming system as the HTML files. A bit 
more looseness with the number of subdivision levels is OK since there 
wont be as many of these files.

Bitmap files that are used for general adornment (bullets, dividers, etc.) of HTML files
don't need the naming prefixes.

Use PNG format bitmap files unless there is a real good reason not to. Avoid GIF's and
JPEG's.


HTML Format
-----------

If you're creating new HTML files use an existing one as a template.
This way you'll get all the right style sheets and other stuff.

Use only generic HTML. i.e., Standard header tags (H1, H2, ...). Surround
paragraphs with matching tags (<P>....</P>). etc.

No fancy formatting. The style sheet will take care of the presentation.

Please don't change the main style sheet without asking me first!

Ignore the warnings about alias files not existing. They do. The 
HTML workshop just doesn't understand in-page name tags.

Thanks for any help you provide! (<-- Intentional pun alert!)

-----------
Dale Larson
dlarson@norsesoft.com
9/3/2001