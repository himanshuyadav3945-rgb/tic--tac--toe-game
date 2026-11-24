const cells = document.querySelectorAll('.cell');
const statusText = document.getElementById('status');
const resetButton = document.getElementById('reset-button');

// Modal Elements
const winnerModal = document.getElementById('winner-modal');
const modalBox = document.getElementById('modal-box');
const modalIcon = document.getElementById('modal-icon');
const modalTitle = document.getElementById('modal-title');
const modalSubtitle = document.getElementById('modal-subtitle');
const modalPlayAgain = document.getElementById('modal-play-again');

// Live C++ Server URL on Render
const serverUrl = 'https://tic-tac-toe-game-t8wg.onrender.com';

async function updateBoard() {
  try {
    const response = await fetch(serverUrl + '/state');
    const data = await response.json();
    renderData(data);
  } catch (error) {
    statusText.textContent = 'C++ server is not running';
    statusText.classList.remove('x-turn', 'o-turn');
  }
}

async function makeMove(index) {
  try {
    const response = await fetch(serverUrl + '/move?index=' + index);
    const data = await response.json();
    renderData(data);
  } catch (error) {
    statusText.textContent = 'C++ server is not running';
    statusText.classList.remove('x-turn', 'o-turn');
  }
}

async function resetGame() {
  hideModal();
  try {
    const response = await fetch(serverUrl + '/reset');
    const data = await response.json();
    renderData(data);
  } catch (error) {
    statusText.textContent = 'C++ server is not running';
    statusText.classList.remove('x-turn', 'o-turn');
  }
}

function showModal(statusMessage) {
  const upperStatus = statusMessage.toUpperCase();
  modalBox.classList.remove('x-win', 'o-win');

  if (upperStatus.includes('X')) {
    modalBox.classList.add('x-win');
    modalIcon.textContent = '🏆';
    modalTitle.textContent = 'Player X Wins!';
    modalSubtitle.textContent = 'Cyan dominates the neon grid!';
  } else if (upperStatus.includes('O')) {
    modalBox.classList.add('o-win');
    modalIcon.textContent = '🏆';
    modalTitle.textContent = 'Player O Wins!';
    modalSubtitle.textContent = 'Magenta takes the victory!';
  } else {
    modalIcon.textContent = '🤝';
    modalTitle.textContent = "It's a Draw!";
    modalSubtitle.textContent = 'Well played by both sides!';
  }

  winnerModal.classList.remove('hidden');
}

function hideModal() {
  winnerModal.classList.add('hidden');
}

function renderData(data) {
  // Update each cell with neon X or O glow
  cells.forEach((cell, index) => {
    const mark = (data.board[index] || '').trim();
    cell.textContent = mark;
    cell.classList.remove('x', 'o');

    if (mark.toUpperCase() === 'X') {
      cell.classList.add('x');
    } else if (mark.toUpperCase() === 'O') {
      cell.classList.add('o');
    }
  });

  // Update status text and highlight active player's turn pill
  const currentStatus = data.gamestatus || data.status || 'Game Over';
  statusText.textContent = currentStatus;
  statusText.classList.remove('x-turn', 'o-turn');

  const upperStatus = currentStatus.toUpperCase();

  // Trigger Victory / Draw Pop-up if the game has ended
  if (upperStatus.includes('WIN') || upperStatus.includes('DRAW') || upperStatus.includes('TIE')) {
    showModal(currentStatus);
  } else {
    hideModal();
    if (upperStatus.includes('X')) {
      statusText.classList.add('x-turn');
    } else if (upperStatus.includes('O')) {
      statusText.classList.add('o-turn');
    }
  }
}

// Event Listeners
cells.forEach((cell, index) => {
  cell.addEventListener('click', () => makeMove(index));
});

resetButton.addEventListener('click', resetGame);
modalPlayAgain.addEventListener('click', resetGame);

// Load initial board state from C++ server
updateBoard();
