import React, { useState } from 'react';

const Tictac = () => {
  const [board, setBoard] = useState(Array(9).fill(null));
  const [isXTurn, setIsXTurn] = useState(true);
  const winner = calculateWinner(board);

  const handleClick = (index) => {
    if (board[index] || winner) return;
    const newBoard = board.slice();
    newBoard[index] = isXTurn ? 'X' : 'O';
    setBoard(newBoard);
    setIsXTurn(!isXTurn);
  };

  const handleReset = () => {
    setBoard(Array(9).fill(null));
    setIsXTurn(true);
  };

  const renderCell = (index) => (
    <div style={styles.cell} onClick={() => handleClick(index)}>
      {board[index]}
    </div>
  );

  return (
    <div style={styles.container}>
      <h1 style={styles.heading}>🎮 Tic Tac Toe</h1>
      <p style={styles.status}>
        {winner
          ? `🏆 Winner: ${winner}`
          : board.every(Boolean)
          ? "🤝 It's a Tie!"
          : `Turn: ${isXTurn ? '❌ X' : '⭕ O'}`}
      </p>
      <div style={styles.board}>
        {board.map((_, i) => renderCell(i))}
      </div>
      <button onClick={handleReset} style={styles.resetButton}>Restart Game</button>
    </div>
  );
};

// Helper function to calculate winner
function calculateWinner(squares) {
  const lines = [
    [0, 1, 2], [3, 4, 5], [6, 7, 8], // rows
    [0, 3, 6], [1, 4, 7], [2, 5, 8], // cols
    [0, 4, 8], [2, 4, 6]             // diagonals
  ];
  for (let [a, b, c] of lines) {
    if (squares[a] && squares[a] === squares[b] && squares[a] === squares[c]) {
      return squares[a];
    }
  }
  return null;
}

const styles = {
  container: {
    textAlign: 'center',
    marginTop: '60px',
    fontFamily: 'Arial, sans-serif',
    color: '#2c3e50',
  },
  heading: {
    fontSize: '2.5rem',
    marginBottom: '10px',
  },
  status: {
    fontSize: '1.4rem',
    marginBottom: '20px',
  },
  board: {
    display: 'grid',
    gridTemplateColumns: '100px 100px 100px',
    gridGap: '10px',
    justifyContent: 'center',
    marginBottom: '20px',
  },
  cell: {
    width: '100px',
    height: '100px',
    backgroundColor: '#ecf0f1',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '2rem',
    cursor: 'pointer',
    borderRadius: '10px',
    boxShadow: '0 2px 5px rgba(0,0,0,0.2)',
    transition: '0.2s',
  },
  resetButton: {
    padding: '10px 20px',
    fontSize: '1rem',
    borderRadius: '8px',
    backgroundColor: '#3498db',
    color: 'white',
    border: 'none',
    cursor: 'pointer',
  },
};

export default Tictac;
