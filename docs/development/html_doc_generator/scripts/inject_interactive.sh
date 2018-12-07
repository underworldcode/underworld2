#! /usr/bin/env bash


set -e
# copy in juniper javascript and stylesheet
cp juniper.min.js ../www/js/.
cp material.css ../www/js/.
cd ../www/UserGuide/
# set code cells to interactive
find . -name \*.html | xargs sed -i.bak 's/\<pre\>\<code class\=\"python\"\>/\<pre data\-executable\>/g'
# strip <code> markup from outputs (where they exist)
find . -name \*.html | xargs sed -i.bak 's/\<pre\>\<code\>/\<pre\>/g'
# replace closing tags to just <pre>
find . -name \*.html | xargs sed -i.bak 's/\<\/code\>\<\/pre\>/\<\/pre\>/g'
# remove final </html> and </body> as they'll be added again in the next step
find . -name \*.html | xargs sed -i.bak 's/\<\/body\>//g'
find . -name \*.html | xargs sed -i.bak 's/\<\/html\>//g'
# make bash variable to append to html
VAR=$(cat <<'END_HEREDOC'
        <script src="../js/juniper.min.js"></script>
        <script>
            new Juniper({
                repo: 'underworldcode/underworld2-documentation',
                isolateCells: false,
                theme: 'material',
                msgLoading: "Done"
            });
        </script>
</body>
<head>
  <link rel="stylesheet" href="../js/material.css" />
</head>
</html>
END_HEREDOC
)

echo "$VAR" | tee -a *.html >/dev/null