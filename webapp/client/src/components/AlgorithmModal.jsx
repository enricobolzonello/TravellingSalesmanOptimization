import Box from '@mui/material/Box';
import Modal from '@mui/material/Modal';
import IconButton from '@mui/material/IconButton';
import { useState } from 'react';
import MarkdownRenderer from './MarkdownRenderer';
import InfoIcon from '@mui/icons-material/Info';

import PropTypes from 'prop-types';

const style = {
  position: 'absolute',
  overflowY: 'scroll',
  maxHeight: "90%",
  top: '50%',
  left: '50%',
  transform: 'translate(-50%, -50%)',
  width: 700,
  bgcolor: 'background.paper',
  border: '2px solid #000',
  boxShadow: 24,
  p: 4,
};

AlgorithmModal.propTypes = {
  algorithm: PropTypes.string
}

function getContent(a){
  switch(a){
    case "0":
      return(
        <MarkdownRenderer filePath={"./markdown/greedy.md"}/>
      )
    default:
      return(
        <p>default</p>
      )
  }
}

export default function AlgorithmModal({algorithm}) {
  const [open, setOpen] = useState(false);
  const handleOpen = () => setOpen(true);
  const handleClose = () => setOpen(false);

  return (
    <div>
      <IconButton onClick={handleOpen} aria-label='info' size='large' sx={{color: '#9CA3AF', "&:hover": { color: "#21d4fc" }}}>
        <InfoIcon />
      </IconButton>
      <Modal
        open={open}
        onClose={handleClose}
        aria-labelledby="modal-modal-title"
        aria-describedby="modal-modal-description"
      >
        <Box sx={style}>
          {getContent(algorithm)}
        </Box>
      </Modal>
    </div>
  );
}