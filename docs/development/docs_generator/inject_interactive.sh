#! /usr/bin/env bash


set -e
# copy in juniper javascript and stylesheet
#cp juniper.min.js ../www/js/.
#cp material.css ../www/js/.
cd ${1}/user_guide
# set code cells to interactive
find . -name \*.html | xargs sed -i 's/<div class="highlight-TOREPLACE notranslate"><div class="highlight"><pre>/<div><div><pre data-executable>/g'
## strip <code> markup from outputs (where they exist)
#find . -name \*.html | xargs sed -i 's/\<pre\>\<code\>/\<pre\>/g'
## replace closing tags to just <pre>
#find . -name \*.html | xargs sed -i 's/\<\/code\>\<\/pre\>/\<\/pre\>/g'
# remove final </html> and </body> as they'll be added again in the next step
find . -name \*.html | xargs sed -i 's/\<\/body\>//g'
find . -name \*.html | xargs sed -i 's/\<\/html\>//g'
# make bash variable to append to html
VAR=$(cat <<'END_HEREDOC'
        <script src="../_static/juniper.min.js"></script>
        <script>
            new Juniper({
                repo: 'underworldcode/underworld2',
                branch: 'GITCOMMIT',
                isolateCells: false,
                theme: 'material',
                msgLoading: "Done"
            });
        </script>
</body>
<head>
  <link rel="stylesheet" href="../_static/css/material.css" />
</head>
</html>
END_HEREDOC
)

echo "$VAR" | tee -a *.html >/dev/null

# get current git commit
export COMMIT=`git rev-parse --verify HEAD`
find . -name \*.html | xargs sed -i "s/GITCOMMIT/${COMMIT}/g"

