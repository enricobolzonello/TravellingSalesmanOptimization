import { useState, useEffect } from 'react';
import {MathpixMarkdown, MathpixLoader} from 'mathpix-markdown-it';
import { loadSre } from "mathpix-markdown-it/lib/sre/sre-browser";


import PropTypes from 'prop-types';

const outMath = {
  include_svg: true,
  // Show in context menu:
  include_smiles: true,
  include_asciimath: true,
  include_latex: true,
  include_mathml: true,
  include_mathml_word: true,
};

let accessibility = {
  assistiveMml: true,
  sre: loadSre()
};

MarkdownRenderer.propTypes = {
    filePath: PropTypes.string
}

function MarkdownRenderer({ filePath }) {
  const [markdownContent, setMarkdownContent] = useState(null);

  useEffect(() => {
    if(filePath != ""){
        import(filePath)
        .then(res => {
            fetch(res.default)
                .then(res => res.text(),console.log(res))
                .then(res => setMarkdownContent(res))
                .catch(err => console.log(err));
        })
        .catch(err => console.log(err));
    }
});

  return (
    <>
      {markdownContent && 
      <MathpixLoader>
          <MathpixMarkdown text="\\(ax^2 + bx + c = 0\\)"
                            outMath={outMath}
                            accessibility={accessibility}
          />
      </MathpixLoader>}
    </>
  );
}

export default MarkdownRenderer;
